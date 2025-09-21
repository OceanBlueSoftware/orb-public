#!/bin/bash

set -euxo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <polyfill|moderator|message_pipe>"
    exit 1
fi

TARGET_PATH=""

# Define allowed values
case "$1" in
    polyfill|moderator)
        echo "Valid input: $1"
	TARGET_PATH="third_party/orb/orblibrary"
        ;;
    message_pipe)
        echo "Valid input: $1"
        TARGET_PATH="third_party/blink/renderer/modules/orb"
        ;;
    *)
        echo "Invalid input. Allowed values: polyfill, moderator, message_pipe"
        exit 1
        ;;
esac

ROOT_DIR=${HOME}/source/repos/chromium_122_0_6261_171

# Android build directory
BUILD_DIR=$ROOT_DIR/out/Default

pushd $ROOT_DIR/src
echo $PWD

TEST_EXE="test_orb_${1}"
echo "Building $TARGET_PATH:$TEST_EXE"

autoninja -C $BUILD_DIR $TARGET_PATH:$TEST_EXE

DEST_ADB_DIR="/data/local/tmp"

adb push $BUILD_DIR/libtest_trace_processor.so $DEST_ADB_DIR
adb push $BUILD_DIR/libthird_party_abseil-cpp_absl.cr.so $DEST_ADB_DIR
adb push $BUILD_DIR/libbase.cr.so $DEST_ADB_DIR
adb push $BUILD_DIR/libbase_allocator_partition_allocator_src_partition_alloc_allocator_shim.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/libboringssl.cr.so $DEST_ADB_DIR # Doesn't exist!!
# adb push $BUILD_DIR/libjsoncpp.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/libgtest.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/libgtest_main.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/libgmock.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/libgmock_main.cr.so $DEST_ADB_DIR
# adb push $BUILD_DIR/test_orb_$1 $DEST_ADB_DIR

adb shell chmod +x $DEST_ADB_DIR/$TEST_EXE

adb shell "LD_LIBRARY_PATH=$DEST_ADB_DIR $DEST_ADB_DIR/$TEST_EXE"

popd
