#!/usr/bin/env bash
# compare fsrc with ripgrep

function green  {
    echo -e "\033[1;32m$1\033[0m"
}

SEARCH_DIR="libs/boost/include"
TERM="filesystem"
TIMEFORMAT="%R"

echo "Searching in $SEARCH_DIR for $TERM"

echo
green "===FSRC==="
echo "./fsrc -t \"$TERM\" --no-colors -d \"$SEARCH_DIR\""
time ./fsrc -t "$TERM" --no-colors -d "$SEARCH_DIR" | tail -n 2 | grep .
echo

green "====RG===="
echo "rg --stats -u -e \"$TERM\" \"$SEARCH_DIR\""
time rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 8
echo

green "===FIND==="
echo "find \"$SEARCH_DIR\" -type f | wc -l"
time find "$SEARCH_DIR" -type f | wc -l
echo
