#!/bin/bash

OUTPUT="$(/usr/bin/geteth.o `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g` )"

echo "${OUTPUT}"

ETHERCAT=$(grep Ethercat $(OUTPUT))
echo $ETHERCAT


