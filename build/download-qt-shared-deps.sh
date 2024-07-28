#!/usr/bin/env bash

# Download Clang and LLVM
apt-get download $(apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts --no-breaks --no-replaces --no-enhances clang-$CLANG_VERSION llvm-$CLANG_VERSION llvm-$CLANG_VERSION-runtime libclang-rt-$CLANG_VERSION-dev libclang1-$CLANG_VERSION lld-$CLANG_VERSION libc++-$CLANG_VERSION-dev libc++abi-$CLANG_VERSION-dev libunwind-$CLANG_VERSION-dev libclang-common-$CLANG_VERSION-dev libclang-$CLANG_VERSION-dev libclang1-$CLANG_VERSION clang-format-$CLANG_VERSION python3-clang-$CLANG_VERSION clangd-$CLANG_VERSION clang-tidy-$CLANG_VERSION | grep "^\w" | sort -u)

# Download General Build Tools
apt-get download $(apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts --no-breaks --no-replaces --no-enhances build-essential cmake ninja-build ccache git python3 perl | grep "^\w" | sort -u)

# Download Required Dependency Libraries
apt-get download $(apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts --no-breaks --no-replaces --no-enhances libasound2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libxrandr-dev | grep "^\w" | sort -u)
apt-get download $(apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts --no-breaks --no-replaces --no-enhances libpulse-dev ffmpeg | grep "^\w" | sort -u)

exit 0
