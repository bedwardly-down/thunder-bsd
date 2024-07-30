#!/usr/bin/env bash

# Install Clang and LLVM
sudo apt-get install -y clang-$CLANG_VERSION llvm-$CLANG_VERSION llvm-$CLANG_VERSION-runtime libclang-rt-$CLANG_VERSION-dev libclang1-$CLANG_VERSION lld-$CLANG_VERSION libc++-$CLANG_VERSION-dev libc++abi-$CLANG_VERSION-dev libunwind-$CLANG_VERSION-dev libclang-common-$CLANG_VERSION-dev libclang-$CLANG_VERSION-dev libclang1-$CLANG_VERSION clang-format-$CLANG_VERSION python3-clang-$CLANG_VERSION clangd-$CLANG_VERSION clang-tidy-$CLANG_VERSION

# Install General Build Tools
sudo apt-get install -y build-essential ccache cmake ninja-build git python3 perl libedit-dev g++-12 libstdc++-12-dev libstdc++-11-dev

# Install Required Build Dependencies
sudo apt-get install -y libasound2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrandr-dev
sudo apt-get install -y libpulse-dev ffmpeg

exit 0
