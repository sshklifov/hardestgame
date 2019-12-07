#!/bin/bash

SUBDIRS=$(find . -maxdepth 1 -not -path . -type d)

for d in $SUBDIRS; do
    cd $d
    make -f ../Modules.mk headers
    cd ../
done

if [ "$1" == "h" ]; then
    exit
fi

if [ "$1" == "f" ]; then
    opt="-B"
fi

for d in $SUBDIRS; do
    cd $d

    make $opt -f ../Modules.mk objs
    if [ $? != 0 ] ; then
        exit
    fi

    cd ../
done

if [ "$1" == "o" ]; then
    exit
fi

make
