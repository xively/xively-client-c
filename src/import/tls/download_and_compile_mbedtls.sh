#!/bin/bash
echo
echo "---------------"
echo "mbedTLS LICENSE"
echo "---------------"
echo "Unless specifically indicated otherwise in a file, files are licensed"
echo "under the Apache 2.0 license, as can be found in: apache-2.0.txt"
echo
echo "the apache-2.0.txt file can be accessed here:"
echo "   https://github.com/ARMmbed/mbedtls/blob/development/apache-2.0.txt"
echo
echo "Do you wish to download mbedTLS from github? (Y)"
read -p "TLS Download Response => " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo
    echo "exiting build.  run make help_tls for more information."
    echo
    exit 1
fi
git clone https://github.com/ARMmbed/mbedtls.git
cd mbedtls
git checkout tags/mbedtls-2.3.0
# "-O2" comes from mbedtls/library/Makefile "CFLAGS ?= -O2" define
make CFLAGS="-O2 -DMBEDTLS_PLATFORM_MEMORY"
cd ..
echo "mbedTLS Build Complete."

