#!/bin/bash

OUTPUT="$(geteth `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g` )"

echo "${OUTPUT}"

# ETHERCAT=`geteth | grep -oP "Ethercat stack connected to \K(.+)"
 # grep Ethercat $(OUTPUT)
# echo "${ETHERCAT}"



