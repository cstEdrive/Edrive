#!/bin/bash

# INSTALL SCRIPT FOR EDRIVE

USREXISTS=0
for i in $(cut -d: -f1 /etc/passwd); do
	if [ $i == 'ebox' ]; then
		USREXISTS=1
	fi;
done

if [ USREXISTS == 0 ]; then
	sudo adduser ebox --home /home/ebox --disabled-password --gecos ''
	sudo usermod -a -G adm,cdrom,sudo,dip,plugdev,lpadmin,sambashare ebox
fi

apt-get update -qq

apt-get install -yy software-properties-common
add-apt-repository ppa:webupd8team/java

apt-get update -qq

apt-get install -yy htop gdebi synaptic grsync bleachbit gufw

apt-get install -yy vim \
							libpq-dev \
							net-tools \
							software-properties-common \
							git \
							rpcbind \
							nfs-common \
							network-manager-pptp \
							libgtk2.0-dev \
							oracle-java8-installer \
							gedit \
							ssh \
							make \
							libpq-dev \
							ethtool \
							postgresql \
							postgresql-client -y




chown -R root:root /home/ebox/Edrive
chmod -R 755 /home/ebox/Edrive



mkdir /usr/src/soem

CURRENT_DIR=$(pwd)
SRC_DIR=$CURRENT_DIR/src

cd $SRC_DIR/Libs/Timer_posix_AK/
./buildlib

cd $SRC_DIR/Libs/soem
cp -Rf * /usr/src/soem/
cd /usr/src/soem
make -s clean
make -s all

cd $SRC_DIR/Libs/soem
make -s clean
make -s all

cd $SRC_DIR/Libs/ec
./buildlib

cd $SRC_DIR/Libs/Coin
./buildlib

cd $SRC_DIR/Scripts/
./buildgeteth

cp geteth.o /usr/bin/geteth.o
rm /usr/bin/geteth
ln -s $SRC_DIR/Scripts/geteth.sh /usr/bin/geteth

# ============================================== #

ESCOPE_DIR=$SRC_DIR/Apps/E-Scope


cd $ESCOPE_DIR/incomm/
chmod a+x buildlib
./buildlib

cd $ESCOPE_DIR/div/
chmod a+x buildlib
./buildlib

cd $ESCOPE_DIR/remez/
chmod a+x buildlib
./buildlib

cd $ESCOPE_DIR/fir/
chmod a+x buildlib
./buildlib

cd cd $ESCOPE_DIR/fungen/
chmod a+x buildlib
./buildlib

cd $ESCOPE_DIR/engine/
chmod a+x buildlib
chmod a+x buildeseng
./buildlib
./buildeseng

cd $ESCOPE_DIR/gui/EscopeGTK/
chmod a+x Escope_build
chmod a+x Escope_run
./Escope_build

cd $SRC_DIR

echo "Modifications to be made manually"

echo "1. Within /home/ebox/svn/src/E-box/Targets/ectarget/ec_main.c"

echo "   change line 560 to:"

echo "   /usr/local/MATLAB/R2015b/rtw/c/ectarget/ectarget.u1"



