cd bsp
rm -rf output
doxygen doxygen.config
cd ../api
rm -rf output
doxygen doxygen.config
cd ..
