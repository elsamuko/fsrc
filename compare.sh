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
        SEARCH_DIR="C:\\ProgramData\\boost"
        ;;
    *)
        echo "Unknown OS"
        ;;
esac


FSRC="$PWD/fsrc"

echo
echo "fsrc"
(cd "$SEARCH_DIR" && ($FSRC [Tt]est | tail -n 1))
echo
echo "rg"
(cd "$SEARCH_DIR" && (rg --stats [Tt]est | tail -n 3 ))
echo

