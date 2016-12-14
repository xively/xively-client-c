#!/bin/bash

set -e

LIBFUZZER_DOWNLOAD_DIR=$1
LIBFUZZER_INSTALL_DIR=$2

mkdir -p $LIBFUZZER_DOWNLOAD_DIR
mkdir -p $LIBFUZZER_INSTALL_DIR

git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer $LIBFUZZER_DOWNLOAD_DIR

cd $LIBFUZZER_DOWNLOAD_DIR
clang++ -c -g -v -O2 -lstdc++ -std=c++11 *.cpp -IFuzzer
ar ruv libFuzzer.a Fuzzer*.o

cp libFuzzer.a $LIBFUZZER_INSTALL_DIR


