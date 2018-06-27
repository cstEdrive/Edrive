#!/bin/bash

geteth `ifconfig -a | sed 's/[ \t].*//;/^$/d' | sed s/://g`


