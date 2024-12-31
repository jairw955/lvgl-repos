#!/bin/bash

cat $1 | grep "LVGL\|LV_" | grep -v "#" > $2
sed -i -e "s/^/set\(/;s/=y/=1/;s/=/ /;s/$/)/;s/\"/\"\\\\\"/;s/\")/\\\\\"\")/" $2
