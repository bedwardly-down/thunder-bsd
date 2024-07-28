#!/usr/bin/env bash

# Install General Qt6 Development Libraries
sudo apt-get install -y qt6-base-dev qt6-base-private-dev qt6-declarative-dev qt6-declarative-private-dev qt6-tools-dev qt6-tools-private-dev qt6-scxml-dev qt6-documentation-tools libqt6core5compat6-dev qt6-tools-dev-tools qt6-l10n-tools qt6-shader-baker libqt6shadertools6-dev qt6-quick3d-dev qt6-quick3d-dev-tools libqt6svg6-dev libqt6quicktimeline6-dev libqt6serialport6-dev libqt6opengl6-dev

sudo apt-get build-dep -y qml6-module-qt5compat-graphicaleffects

exit 0
