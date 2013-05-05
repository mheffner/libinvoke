#!/bin/sh

if [ "$#" != "1" ]; then
    echo "Usage: symdump.sh executable"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "$1 does not exist"
    exit 1
fi

nm -n $1 | grep -E '^[0-9a-fA-F]+ (T|t) ' | sed 's/ [Tt] / /' | grep -vE 'gcc[23]_compiled\.$'
