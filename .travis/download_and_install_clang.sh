#!/bin/bash
set -e 

CLANG_REPOSITORY_URL=https://chromium.googlesource.com/chromium/src/tools/clang
CLANG_DOWNLOAD_DIR=`pwd`/clang_download_dir

mkdir -p $CLANG_DOWNLOAD_DIR
cd $CLANG_DOWNLOAD_DIR 

echo "Downloading clang..."
git clone $CLANG_REPOSITORY_URL

echo "Installing clang...."
$CLANG_DOWNLOAD_DIR/clang/scripts/update.py 

mv ../third_party $CLANG_DOWNLOAD_DIR 

echo "Testing installation..."
$CLANG_DOWNLOAD_DIR/third_party/llvm-build/Release+Asserts/bin/clang --version


