#!/bin/bash

make -f ../Makefile clean
eval make -j 4 -f ../Makefile gnu_osx_64

