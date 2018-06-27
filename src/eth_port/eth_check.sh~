#!/bin/bash

#This script checks whether the port name of the computer is set to eth0, eth1 or eth2.
#If this is not the case, it changes it to eth0.
#This script is called from qadscope.m and requires reboot if changes are made to the port number.
#User must be root to make changes.

# LOG
# M. Plantinga (17 February 2015): file creation.
# M. Plantinga (02 March 2015): eth0 misspelled.

# obtaining eth name and mac address

filepath='/home/ebox/svn/trunk/src/E-box/eth_port'

ifconfig > $filepath/ifconfig.txt                    		#make .txt file of it
grep "eth" $filepath/ifconfig.txt > $filepath/ethernet		#extract the line that contains "eth" out of ifconfig.txt
string_var=$(<$filepath/ethernet)                    	  	#set it as variable
eth_var=$(echo $string_var | cut -c1-4)              		#extract the eth port name and store it as variable
mac_var=$(echo $string_var | cut -c33-49)            	      	#extract the mac address and store it as variable


echo $eth_var $mac_var

# make log.txt file if nonexistent
if [ ! -f $filepath/log.txt ]; then
touch $filepath/log.txt
fi

# check if the ethernet port is correctly named

if [ $eth_var == "eth0" ];
then
return_var='0'
change_stat='unchanged'

elif [ $eth_var == "eth1" ];
then
return_var='0'
change_stat='unchanged'

elif [ $eth_var == "eth2" ];
then
return_var='0'
change_stat='unchanged'


# if port name not equal to eth0/1/2, rename port to eth0
else
echo
echo
echo "RENAMING ETHERNET PORT... you will have to reboot afterwards"
sleep 2

# copy port naming file
cp /etc/udev/rules.d/70-persistent-net.rules $filepath/eth_settings

# change access permission to file
# original settings: -rw-r--r-- 1 root root 816 Feb 13 10:38 70-persistent-net.rules
chmod 777 $filepath/eth_settings

# extract the line that contains the mac address and the old ethernet port name: eth_line_old
grep $mac_var $filepath/eth_settings > $filepath/eth_line_old

# copy and rename eth_line_old to eth_line_new and replace the old ethernet port name with eth0
cp $filepath/eth_line_old $filepath/eth_line_new
sed -i "s/${eth_var}/eth0/g" $filepath/eth_line_new

# create variables of eth_line_old and eth_line_new
var_old_line=$(<$filepath/eth_line_old)
var_new_line=$(<$filepath/eth_line_new)

var_old_line=$(echo "${var_old_line}" | sed -e 's/[]$.*[\^]/\\&/g' )
sed -i -e "s/${var_old_line}/${var_new_line}/g" $filepath/eth_settings

# set access permissions to the original file settings
chmod 755 $filepath/eth_settings
mv $filepath/eth_settings /etc/udev/rules.d/70-persistent-net.rules

change_stat='changed'
return_var='1'
exit 1
fi

echo "$(date) $line $eth_var $change_stat" >> $filepath/log.txt

exit "$return_var"	
