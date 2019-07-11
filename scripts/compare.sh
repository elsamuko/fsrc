#!/usr/bin/env bash
# compare fsrc with ripgrep

function red  {
    echo -e "\033[1;31m$@\033[0m"
}

function green  {
    echo -e "\033[1;32m$@\033[0m"
}

function cyan  {
    echo -e "\033[1;34m$@\033[0m"
}

SEARCH_DIR="libs/boost/include"
TERM="filesystem"
REGEX="fil.{1}system"
TIMEFORMAT="%R"

cyan "Searching in $SEARCH_DIR for $TERM"

echo
green "===FSRC==="
echo "./fsrc -t \"$TERM\" --no-piped --no-colors -d \"$SEARCH_DIR\""
{ time ./fsrc -t "$TERM" --no-piped --no-colors -d "$SEARCH_DIR" | tail -n 2 | grep . ;} 2> tmp/time
red $(cat tmp/time)
echo

echo "./fsrc -r -t \"$REGEX\" --no-piped --no-colors -d \"$SEARCH_DIR\""
{ time ./fsrc -r -t "$REGEX" --no-piped --no-colors -d "$SEARCH_DIR" | tail -n 2 | grep . ;} 2> tmp/time
red $(cat tmp/time)
echo

green "====RG===="
echo "rg --stats -uu --fixed-strings --case-sensitive \"$TERM\" \"$SEARCH_DIR\""
{ time rg --stats -uu --fixed-strings --case-sensitive "$TERM" "$SEARCH_DIR" | tail -n 8 ;} 2> tmp/time
red $(cat tmp/time)
echo

echo "rg --stats -uu --regexp \"$REGEX\" \"$SEARCH_DIR\""
{ time rg --stats -uu --regexp "$REGEX" "$SEARCH_DIR" | tail -n 8 ;} 2> tmp/time
red $(cat tmp/time)
echo

green "===FIND==="
echo "find \"$SEARCH_DIR\" -type f | wc -l"
{ time find "$SEARCH_DIR" -type f | wc -l ;} 2> tmp/time
red $(cat tmp/time)
echo
