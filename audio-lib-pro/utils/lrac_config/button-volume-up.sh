#!/bin/sh

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 port" >&2
    exit 1
fi

echo Send Button Volume Up to $1
echo ./lrac_config.exe -d $1 -v 1 -b 3000000 --button 3
./lrac_config.exe -d $1 -v 1 -b 3000000 --button 3

echo
