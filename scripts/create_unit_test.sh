#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $(basename "$0") MODULE"
    exit 1
fi

MODULE="$1"
TESTMODULE="Test$MODULE"

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MAIN_DIR="$SCRIPT_DIR/.."
cd $MAIN_DIR

mkdir -p test
cd test

if [ -d "$TESTMODULE" ]; then
    echo "$TESTMODULE already exists"
    exit 1
fi

mkdir "$TESTMODULE"
mkdir -p "$TESTMODULE/qmake"
mkdir -p "$TESTMODULE/src"

#
# generate unit test project
#
cat > "$TESTMODULE/qmake/$TESTMODULE.pro" <<EOL
CONFIG += static

MAIN_DIR=../../..
PRI_DIR=\$\${MAIN_DIR}/qmake

include( \$\${PRI_DIR}/setup.pri )
linux: include( \$\${PRI_DIR}/linux.pri )
win32: include( \$\${PRI_DIR}/win.pri )
macx:  include( \$\${PRI_DIR}/mac.pri )

include( \$\${PRI_DIR}/unit_test.pri )
include( \$\${PRI_DIR}/boost.pri )

# testsuite
SOURCES += ../src/$TESTMODULE.cpp

EOL

#
# generate unit test source
#
cat > "$TESTMODULE/src/${TESTMODULE}.cpp" <<EOL
#define BOOST_TEST_MODULE $MODULE

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( $MODULE )

BOOST_AUTO_TEST_CASE( Test_$MODULE ) {
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END()

EOL
