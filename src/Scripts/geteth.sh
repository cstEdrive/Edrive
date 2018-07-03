#!/bin/bash

/usr/bin/geteth.o `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g` | grep Ethercat


