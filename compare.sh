#!/usr/bin/env bash

FSRC="$PWD/fsrc"

echo
echo "fsrc"
(cd "/usr/include/boost/" && ($FSRC filesystem | tail -n 1))
echo
echo "rg"
(cd "/usr/include/boost/" && (rg --stats filesystem | tail -n 3 ))
echo

