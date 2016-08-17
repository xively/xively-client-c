#!/bin/bash

git clone https://github.com/wolfSSL/wolfssl
cd wolfssl
git checkout tags/v3.9.6
(autoreconf --install && ./configure `cat ../wolfssl.conf` && make)
echo "Done"

