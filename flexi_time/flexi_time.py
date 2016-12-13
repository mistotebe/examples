#!/usr/bin/env python
from functools import partial
from datetime import datetime, timedelta

import sys
import erppeek

# Naughty!
last_clean = None
changes = []


def grouping(source, attr='date'):
    current = None
    accum = []
    for item in source:
        if current == item[attr]:
            accum.append(item)
        else:
            yield (current, accum)
            current = item[attr]
            accum = [item]
    yield (current, accum)

def valuate(entry, attr='duration', sign=lambda x: 1):
    return sign(entry) * entry[attr]

def in_lieu_check(client):
    account_ids = client.model('account.analytic.account').search([('name','like','in lieu')])
    def is_in_lieu(entry):
        if entry['account_id'][0] in account_ids:
            return 0
        return 1
    return is_in_lieu

def expire(entries, entry, duration=timedelta(days=30)):
    def carve(amount):
        assert entries

        date, offset = entries[0]
        if amount * offset > 0:
            # append
            return None

        return amount + offset

    #print "Processing {}".format(entry)
    current_date, current_offset = entry

    if abs(sum([x[1] for x in entries])) < 1./8:
        global last_clean
        last_clean = current_date

    if not entries and current_offset:
        entries.append(entry)
        changes.append(entry)
        #print "Adding {} to empty entries".format(entry)
        return entries

    while entries and entries[0][0] + duration < current_date:
        #print "Expiring {1} hours from {0}".format(*entries[0])
        changes.append((current_date, 'Expiration of %s' % (entries[0],)))
        entries.pop(0)

    if current_offset:
        changes.append(entry)

    while entries:
        if not current_offset:
            break

        #print "Starting shave of {} hours from {} hours:".format(current_offset, sum([x[1] for x in entries])),
        new_offset = carve(current_offset)
        if new_offset is None:
            # nothing to shave off - everything has the same sign
            #print "same sign entries"
            entries.append((current_date, current_offset))
        elif new_offset * current_offset < 0:
            # there is still new_offset left in the first entry
            #print "{} hours left in first entry on {}".format(new_offset, current_date)
            date, offset = entries[0]
            entries[0] = (date, new_offset)
        else:
            # we have exhausted the first entry, try with another
            current_offset = new_offset
            #print "exhausted entry of {2} hours from {1} on {0}".format(current_date, *entries[0])
            entries.pop(0)
            continue
        break
    else:
        if current_offset:
            # nothing to shave off, the queue was cleaned
            #print "There was nothing to shave adding {} hours".format(current_offset)
            entries.append((current_date, current_offset))
    return entries

if __name__ == '__main__':
    if len(sys.argv[1:]) < 3:
        print >>sys.stderr, "Too few arguments"
        print >>sys.stderr, "{} URL db username [start_day]".format(sys.argv[0])
    client = erppeek.Client(*sys.argv[1:4])
    ts = client.model('hr.analytic.timesheet')

    domain = [('user_id', '=', client.user)]
    if len(sys.argv) >= 5:
        domain.append(('date', '>=', sys.argv[4]))

    days = list(grouping(ts.read(domain, 'duration date account_id', order='date')))
    sign_check = in_lieu_check(client)
    d = [(datetime.strptime(date, '%Y-%m-%d').date(), sum(map(partial(valuate, sign=sign_check), entries), -8)) for (date, entries) in days if date]
    left = reduce(expire, d, [])
    print "Last 10 changes:"
    for change in changes[-10:]:
        print "\t", change
    if last_clean:
        print "Last clean day was {}, you can use it next time you run this to speed things up".format(last_clean.strftime('%Y-%m-%d'))
    print "Have {} hours to use".format(sum([x[1] for x in left]))
    print left
