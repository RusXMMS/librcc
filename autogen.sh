#!/bin/bash


rm -f configure
aclocal
autoheader
libtoolize --force
automake
autoconf