#!/usr/bin/env bash

MAIN_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

case $(uname) in
    Linux)
        OS=linux
        MAKE="make"
        ;;
    Darwin)
        OS=mac
        MAKE="make"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        OS=win
        OS_EXT=Win
        EXT=".exe"
        MAKE="$MAIN_DIR/utils/jom/jom.exe"
        ;;
    *)
        echo "Error: Unknown OS."
        exit 1
        ;;
esac

TMP_DIR="$MAIN_DIR/tmp/deploy"
QMAKE="$MAIN_DIR/utils/qmake/bin/qmake_$OS$EXT"
GIT_TAG=$(git describe --abbrev=0)
ZIP_FILE="fsrc-$OS-$GIT_TAG.zip"
FSRC_BIN="$MAIN_DIR/fsrc$EXT"
HELPER="$MAIN_DIR/qmake/compile.bat"

function createHelper {
    local VERSIONS=("2019")
    local EDITIONS=("BuildTools" "Community" "Professional" "Enterprise")
    
    echo -e '@echo off\r\n' > "$HELPER"

    for VERSION in "${VERSIONS[@]}"; do
        for EDITION in "${EDITIONS[@]}"; do
            local VCVARS_DIR="C:/Program Files (x86)/Microsoft Visual Studio/$VERSION/$EDITION/VC/Auxiliary/Build"
            if [ -d "$VCVARS_DIR" ]; then
                export VSNEWCOMNTOOLS="${VCVARS_DIR////\\}"
                break 2
            fi
        done
    done

    # VS 2019 Professional/BuildTools/Community
    if [ -n "$VSNEWCOMNTOOLS" ]; then
        echo -ne "call \"$VSNEWCOMNTOOLS\\\\vcvars64.bat\"" >> "$HELPER"
    else
        echo "Could not find Visual Studio 2019"
        exit 1
    fi
    
    echo -e ' > nul\r\n' >> "$HELPER"
    echo -e 'echo %*' >> "$HELPER"
    echo -e '%*' >> "$HELPER"
    chmod +x "$HELPER"
}

function please_remove {
    if [ -f "$1" ]; then rm "$1"; fi
}

function prepare {
    please_remove "$FSRC_BIN"
    please_remove "$ZIP_FILE"
    please_remove "$HELPER"
    mkdir -p "$TMP_DIR"
}

function buildBoost {
    if [ ! -d "$MAIN_DIR/libs/boost" ]; then
        echo "Building boost"
        ./scripts/build_boost.sh
    fi
}

function buildFsrc {
    ( 
        cd qmake || exit 1
        "$QMAKE" fsrc.pro &> "$TMP_DIR/qmake.log"
        "$MAKE" clean &> "$TMP_DIR/make.log"
        "$MAKE" &> "$TMP_DIR/make.log"
    )
}

function buildFsrcWin {
    createHelper
    ( 
        cd qmake || exit 1
        "$HELPER" "$(cygpath -w "$QMAKE")" fsrc.pro &> "$TMP_DIR/qmake.log"
        "$HELPER" "$(cygpath -w "$MAKE")" clean &> "$TMP_DIR/make.log"
        "$HELPER" "$(cygpath -w "$MAKE")" &> "$TMP_DIR/make.log"
    )
}

function check {
    if [ ! -f "$FSRC_BIN" ]; then
        echo "Failed to build fsrc"
        exit 1
    fi
}

function package {
    zip -j "$ZIP_FILE" "$FSRC_BIN" > "$TMP_DIR/zip.log"
}

function please {
    echo "$1"
    "$1"
}

echo "Deploying fsrc $GIT_TAG to $ZIP_FILE"
please prepare
please buildBoost
please "buildFsrc$OS_EXT"
please check
please package
