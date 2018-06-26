#!/bin/bash

# INSTALL SCRIPT FOR EDRIVE

USREXISTS=0
for i in $(cut -d: -f1 /etc/passwd); do
	if [ $i == 'ebox']; then
		USREXISTS=1
	fi
done

if [ USREXISTS == 0 ]; then
	sudo adduser ebox
fi

sudo apt-get update -qq
sudo apt-get install -yy htop gdebi synaptic grsync bleachbit gufw

sudo apt-get install -yy 	libpq-dev \
							net-tools \
							software-properties-common \
							git \
							rpcbind \
							nfs-common \
							network-manager-pptp \
							libgtk2.0-dev \
							oracle-java6-installer \
							gedit \
							ssh \
							make \
							libpq-dev \
							ethtool \
							postgresql \
							postgresql-client -y




chown -R root:root /home/ebox/svn

chmod -R 755 /home/ebox/svn




banner "INSTALLATION COMPLETE"




echo "Modifications to be made manually"

echo "1. Within /home/ebox/svn/trunk/src/E-box/Targets/ectarget/ec_main.c"

echo "   change line 560 to:"

echo "   /usr/local/MATLAB/R2015b/rtw/c/ectarget/ectarget.u1"



