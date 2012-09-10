#!/usr/bin/env python
import curses

def test(stdscr):
    while 1:
        c = stdscr.getch()
        if c == 27:
            stdscr.addstr(" ^[")
            #print "^["
        elif c == 10:
            stdscr.addstr("RET")
        else:
            #if c >= 256:
            #    stdscr. addstr("^")
            #    c -= 256
            try:
                stdscr.addstr("%c" % c)
            except OverflowError:
                stdscr.addstr("Outside range: %d" % c)
    stdscr.refresh()

try:
    curses.wrapper(test)
except KeyboardInterrupt:
    pass
