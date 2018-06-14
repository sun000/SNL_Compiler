#!/bin/bash

if [ ! -d "Parse" ] ; then
    make
fi
./Parse
