#!/bin/bash

OUTPUT="$(/usr/bin/geteth.o `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g` | grep Ethercat)"

echo "${OUTPUT}"


