#!/bin/sh
./configure --with-qt-dir=/usr/lib/qt2 --prefix=/usr
./mk-ui.sh
make
