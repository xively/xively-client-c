#!/bin/bash

set -e

LIBFUZZER_DOWNLOAD_DIR=$1
LIBFUZZER_INSTALL_DIR=$2

mkdir -p $LIBFUZZER_DOWNLOAD_DIR
mkdir -p $LIBFUZZER_INSTALL_DIR

echo "downloading libfuzz..."

git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer $LIBFUZZER_DOWNLOAD_DIR

echo "compiling libfuzz..."

cd $LIBFUZZER_DOWNLOAD_DIR
clang++ -c -g -O2 -lstdc++ -std=c++11 *.cpp -IFuzzer
ar ruv libFuzzer.a Fuzzer*.o

echo "installing libfuzz..."

cp libFuzzer.a $LIBFUZZER_INSTALL_DIR

echo "done"


