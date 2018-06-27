#!/bin/bash

./geteth2 `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g`


