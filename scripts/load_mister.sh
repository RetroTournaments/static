#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "usage: $0 <ip_address>"
    exit 1
fi

use_sshpass="no"

for arg in "$@"; do
    if [[ "arg" == "--sshpass" ]]; then
        use_sshpass="yes"
    fi
done

if [[ "$use_sshpass" == "no" ]]; then
    echo "scp ../data/mister/* to $1"
    scp -r ../data/mister/* "root@$1:/media/fat"
else
    echo "scp ../data/mister/* to $1 --sshpass"
    sshpass -p 1 scp -r ../data/mister/* "root@$1:/media/fat"
fi
