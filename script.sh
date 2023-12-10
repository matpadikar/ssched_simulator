#!/bin/sh
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
sudo apt install pkg-config
sudo apt install libfmt-dev
sudo apt install zlib1g-dev
sudo apt install lemon
cd vcpkg
./vcpkg install fmt
./vcpkg install lemon
./vcpkg install spdlog
./vcpkg install nlohmann-json
./vcpkg install catch2
cd ..
cmake -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
make
