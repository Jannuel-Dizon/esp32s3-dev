#!/bin/bash

docker run -it --rm \
    --name ESP32_DevContainer \
    --hostname ESP32-Dev \
    --privileged \
    -p 2224:22 \
    --device=/dev/ttyUSB0 \
    -v "$(pwd)/workspace:/workspace" \
    -w /workspace \
    esp32_devcontainer
