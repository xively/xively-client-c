#!/bin/bash
set -e 

CLANG_REPOSITORY_URL=https://chromium.googlesource.com/chromium/src/tools/clang
CLANG_DOWNLOAD_DIR=$1
CLANG_INSTALL_DIR=$2

mkdir -p $CLANG_DOWNLOAD_DIR
cd $CLANG_DOWNLOAD_DIR 

echo "Downloading clang..."
git clone $CLANG_REPOSITORY_URL

echo "Installing clang...."
python2 clang/scripts/update.py 

mv ../third_party $CLANG_DOWNLOAD_DIR 

echo "Testing installation..."
$CLANG_DOWNLOAD_DIR/third_party/llvm-build/Release+Asserts/bin/clang --version
ln -s $CLANG_DOWNLOAD_DIR/third_party/llvm-build/Release+Asserts $CLANG_INSTALL_DIR

echo "done"
