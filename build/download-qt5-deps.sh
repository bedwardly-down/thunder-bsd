#!/usr/bin/env bash

# Download Qt5 Build Dependencies
apt-get download $(apt-cache depends --recurse --no-recommends --no-suggests --no-conflicts --no-breaks --no-replaces --no-enhances qt5-base-dev qt5-base-private-dev qt5-declarative-dev qt5-declarative-private-dev qt5-tools-dev qt5-tools-private-dev qt5-scxml-dev qt5-documentation-tools libqt5core5compat5-dev qt5-tools-dev-tools qt5-l10n-tools qt5-shader-baker libqt5shadertools5-dev qt5-quick3d-dev qt5-quick3d-dev-tools libqt5svg5-dev libqt5serialport5-dev libqt5opengl5-dev | grep "^\w" | sort -u)

exit 0
