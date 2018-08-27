#!/usr/bin/env bash

TERM="test"
SEARCH_DIR="/usr/include/netinet"
echo "Searching in $SEARCH_DIR for $TERM"

sudo dtruss -eo -c ./fsrc -d "$SEARCH_DIR" --no-git "$TERM" 2> >(grep -v "dtrace:" > dtrace_fsrc.log)
sudo dtruss -eo -c rg -u "$TERM" "$SEARCH_DIR"              2> >(grep -v "dtrace:" > dtrace_rg.log)

