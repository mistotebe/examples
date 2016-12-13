#!/bin/bash
shopt -s extdebug

## abort
# prints the call stack and paramaters of the functions if extdebug was set
# soon enough
#
# if someone mucks with the $@ array (using `set --`), the original arguments
# are lost and only the replaced arguments are printed
abort() {
    local level=0
    local offset=${BASH_ARGC[0]}

    local line func file args argc
    while caller "$level" >/dev/null; do
        line=$(caller "$level" | cut -d' ' -f 1)
        func=$(caller "$level" | cut -d' ' -f 2)
        file=$(caller "$level" | cut -d' ' -f 3-)

        # `caller $x` corresponds to ${BASH_ARGC[x+1]}
        argc="${BASH_ARGC[level+1]}"
        args=( "${BASH_ARGV[@]:$offset:$argc}" )

        #echo "level=$level offset=$offset argc=$argc (${args[@]})"

        echo -n "$func("

        # BASH_ARGV is a stack, so the order of the variables seems reversed
        local index
        for ((index=argc; index > 0; index--)); do
            if [ "$index" -lt "$argc" ]; then
                printf ", "
            fi
            printf "%q" "${args[index - 1]}"
        done
        echo ") at $file:$line"
        let offset+=argc
        let level++;
    done
}

fact() {
    local level=$1
    if [ "$level" -le 0 ]; then
        abort;
    else
        fact "$((level - 1))";
    fi
}

fact "$@"
