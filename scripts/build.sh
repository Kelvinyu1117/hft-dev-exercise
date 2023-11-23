export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
rm -r build
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja -j 10
