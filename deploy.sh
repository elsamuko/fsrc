#!/usr/bin/env bash

MAIN_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

case $(uname) in
    Linux)
        OS=linux
        CMAKE="/snap/bin/cmake" # snap install cmake --classic
        ;;
    Darwin)
        OS=mac
        CMAKE="cmake" # brew install cmake
        ;;
    CYGWIN*|MINGW*|MSYS*)
        OS=win
        OS_EXT=Win
        EXT=".exe"
        CMAKE="$USERPROFILE/scoop/apps/cmake/current/bin/cmake" # scoop install cmake
        ;;
    *)
        echo "Error: Unknown OS."
        exit 1
        ;;
esac

TMP_DIR="$MAIN_DIR/tmp/deploy"
CMAKE_DIR="$MAIN_DIR/cmake"
BUILD_DIR="$CMAKE_DIR/build"
GIT_TAG=$(git describe --abbrev=0)
ZIP_FILE="fsrc-$OS-$GIT_TAG.zip"
FSRC_BIN="$MAIN_DIR/fsrc$EXT"
HELPER="$BUILD_DIR/compile.bat"

function createHelper {
    local VERSIONS=("2019")
    local EDITIONS=("BuildTools" "Community" "Professional" "Enterprise")
    
    echo -ne '@echo off\r\n\r\n' > "$HELPER"

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
    
    echo -ne ' > nul\r\n\r\n' >> "$HELPER"
    echo -ne 'echo %*\r\n' >> "$HELPER"
    echo -ne '%*\r\n' >> "$HELPER"
    chmod +x "$HELPER"
}

function please_remove {
    if [ -f "$1" ]; then rm "$1"; fi
    if [ -d "$1" ]; then rm -r "$1"; fi
}

function prepare {
    please_remove "$FSRC_BIN"
    please_remove "$ZIP_FILE"
    please_remove "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
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
        cd "$BUILD_DIR" || exit 1
        # configure
        "$CMAKE" -S .. &> "$TMP_DIR/cmake.log"

        # build
        "$CMAKE" --build . --parallel --config Release &> "$TMP_DIR/cmake.log"
    )
}

function buildFsrcWin {
    createHelper
    ( 
        cd "$BUILD_DIR" || exit 1

        # configure
        "$HELPER" \
        "$CMAKE" -S .. &> "$TMP_DIR/cmake.log"

        # build
        "$HELPER" \
        "$CMAKE" --build . --parallel --config Release &> "$TMP_DIR/cmake.log"
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
