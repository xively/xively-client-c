#!/bin/bash
echo
echo "!! IMPORTANT !!"
echo
echo "WolfSSL's build system requires autotools to be installed on your host system."
echo "Please install the followng packages before proceeding:"
echo
echo "  MacOSX:"
echo "    brew update"
echo "    brew install autoconf automake libtool"
echo "  Ubuntu:"
echo "    sudo apt-get update"
echo "    sudo apt-get install autoconf"
echo "    sudo apt-get install autotools-dev"
echo "    sudo apt-get install libtool"
echo
echo " For other TLS library options please run make help_tls."
echo
echo " ! If you proceed from this step and the build fails then you will need to: !"
echo "    1: make clean_all"
echo "    2: rm -rf src/import/tls/wolfssl"
echo
read -p "Please acknowledge these tool dependencies. Do you wish to proceed? [Y/N] " -n 1 -r
echo 
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo
    echo "Exiting build."
    echo
    exit 1
fi
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
read -p "Do you agree to the license of this third party library? [Y/N] " -n 1 -r
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
git checkout tags/v3.10.2-stable
(autoreconf --install && ./configure `cat ../wolfssl.conf` && make )
echo "WolfSSL Build Complete."

