#!/bin/bash

if [ "$3" == "" ];then
    echo fonttools subset \$1 --text-file="\$2" --output-file="\$3"
    echo \$1-input file, \$2-subset text file, \$3 output file
    exit
fi
fonttools subset $1 --text-file="$2" --output-file="$3"
