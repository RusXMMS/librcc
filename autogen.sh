#!/bin/bash


rm -f configure
aclocal
libtoolize --force
automake
autoconf