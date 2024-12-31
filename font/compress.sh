#!/bin/bash

if [ "$2" == "" ];then
    echo fonttools ttLib.woff2 compress "\$1" -o "\$2"
    echo \$1-input file, \$2 output file
    exit
fi
fonttools ttLib.woff2 compress "$1" -o "$2"
