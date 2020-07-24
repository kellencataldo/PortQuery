#!/bin/bash

sudo apt-get update
cd $TRAVIS_BUILD_DIR
cmake .
cd test
make


