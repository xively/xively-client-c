#!/bin/bash
echo
echo "-----------------"
echo "WolfSSL LICENSING"
echo "-----------------"
echo "wolfSSL (formerly known as CyaSSL) and wolfCrypt are either licensed for use"
echo "under the GPLv2 or a standard commercial license. For our users who cannot use"
echo "wolfSSL under GPLv2, a commercial license to wolfSSL and wolfCrypt is available."
echo "Please contact wolfSSL Inc. directly at:"
echo
echo "Email: licensing@wolfssl.com"
echo "Phone: +1 425 245-8247"
echo
echo "More information can be found on the wolfSSL website at www.wolfssl.com."
echo
echo "Do you wish to download WolfSSL from github? (Y)"
read -p "TLS Download Response => " -n 1 -r
echo 
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo
    echo "Exiting build.  Run make help_tls for more information."
    echo
    exit 1
fi
git clone https://github.com/wolfSSL/wolfssl
cd wolfssl
git checkout tags/v3.9.6
(autoreconf --install && ./configure `cat ../wolfssl.conf` && make)
echo "WolfSSL Build Complete."

