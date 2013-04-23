#!/bin/bash

DEST_DIR="/usr/share/arduino/libraries"

echo "Script for copying GoL folders to ${DEST_DIR}/libraries"
echo "Call this script standing in GoL's source-folder"
sudo cp -r GameOfLightSim ${DEST_DIR}
sudo cp -r GameOfLightHW ${DEST_DIR}
sudo cp -r utility/ ${DEST_DIR}/GameOfLightHW/
sudo cp -r utility/ ${DEST_DIR}/GameOfLightSim/
