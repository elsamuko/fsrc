#!/usr/bin/env bash
# compare fsrc with ripgrep

case $(uname) in
    Linux)
        SEARCH_DIR="/usr/include/"
        ;;
    Darwin)
        SEARCH_DIR="/usr/include/"
        ;;
    CYGWIN*)
        SEARCH_DIR="$VS140COMNTOOLS..\\..\\VC\\include"
        ;;
    *)
        echo "Unknown OS"
        ;;
esac

TERM="[Tt]est"
echo "Searching in $SEARCH_DIR for $TERM"

echo
echo "fsrc"
./fsrc -t "$TERM" -d "$SEARCH_DIR" | tail -n 1
echo
echo "rg"
rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 3
echo
