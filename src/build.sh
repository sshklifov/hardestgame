#!/bin/bash

SUBDIRS=$(find . -maxdepth 1 -not -path . -type d)

for d in $SUBDIRS; do
    if [ ! -f $d/Makefile ]; then
        cp ../Modules.mk $d/Makefile
    fi
done

for d in $SUBDIRS; do
    cd $d
    make headers
    cd ..
done

if [ "$1" == "c" ]; then
    for d in $SUBDIRS; do
        cd $d
        make clean
        rm Makefile
        cd ..
    done
    make -f ../GNUmakefile clean
    exit
fi

if [ "$1" == "f" ]; then
    opt="-B"
fi
for d in $SUBDIRS; do
    cd $d
    make $opt objs
    if [ $? != 0 ] ; then
        exit
    fi
    cd ..
done

make -f ../GNUmakefile
