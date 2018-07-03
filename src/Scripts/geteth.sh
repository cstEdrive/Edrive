#!/bin/bash

OUTPUT="$(/usr/bin/geteth.o `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g` )"

echo "${OUTPUT}"

echo "${OUTPUT}" | grep -oP "Ethercat stack connected to \K(.+)"
 # grep Ethercat $(OUTPUT)
# echo "${ETHERCAT}"


