#!/bin/sh
set -e
sudo apt-get update
sudo apt-get install -y build-essential pkg-config libpng-dev
make clean
make
