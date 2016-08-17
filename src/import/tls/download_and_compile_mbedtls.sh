#!/bin/bash

git clone https://github.com/ARMmbed/mbedtls.git
cd mbedtls
git checkout tags/mbedtls-2.3.0
# "-O2" comes from mbedtls/library/Makefile "CFLAGS ?= -O2" define
make CFLAGS="-O2 -DMBEDTLS_PLATFORM_MEMORY"
cd ..

echo "Done"

