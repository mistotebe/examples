#!/bin/bash

workdir="$1"
shift
if [ -s "$workdir/.counter" ]; then
    counter="$(< "$workdir/.counter")";
fi
counter="${counter:-1}"

generate_id() {
    id="$(date -uIsecond)_$(( counter++ ))_1"
}

promote() {
    local filelist=( "$workdir/"* )
    local highest_level current_level
    local -a candidates
    local old_file new_file to_remove

    highest_level=$(
        IFS=_
        oldest=( ${filelist[0]} )
        echo "${oldest[2]}"
    )

    for ((current_level=1; current_level <= highest_level; current_level++)); do
        candidates=( "$workdir/"*"_$current_level" )
        # sort candidates properly
        if [ "${#candidates[@]}" -gt 2 ]; then
            old_file=$(basename "${candidates[0]}")
            new_file=$(
                IFS=_
                name_parts=( $old_file )
                let name_parts[2]++
                echo "${name_parts[*]}"
            )
            to_remove="${candidates[1]}"

            mv "$workdir/$old_file" "$workdir/$new_file"
            rm "$to_remove"
            break
        fi
    done
}

for new_file in "$@"; do
    generate_id
    mv "$new_file" "$workdir/$id"
    promote
done

echo "$counter" >"$workdir/.counter"
