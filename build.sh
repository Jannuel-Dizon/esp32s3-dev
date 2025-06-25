#!/bin/bash

docker build \
    -t esp32_devcontainer \
    -f Dockerfile \
    .

ssh-keygen -f '/home/jannnuel-dizon/.ssh/known_hosts' -R '[localhost]:2224'
