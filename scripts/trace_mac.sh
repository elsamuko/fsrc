#!/usr/bin/env bash

TERM="test"
SEARCH_DIR="/usr/include/netinet"
echo "Searching in $SEARCH_DIR for $TERM"

sudo dtruss -e -c ./fsrc -d "$SEARCH_DIR" --no-git "$TERM"  2> dtrace_fsrc.log
sudo dtruss -e -c rg -u $TERM $SEARCH_DIR 2> dtrace_rg.log

