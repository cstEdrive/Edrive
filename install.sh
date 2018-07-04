#!/bin/bash

# INSTALL SCRIPT FOR EDRIVE

USREXISTS=0
for i in $(cut -d: -f1 /etc/passwd); do
	if [ $i == 'edrive' ]; then
		USREXISTS=1
	fi;
done

if [ USREXISTS == 0 ]; then
	sudo adduser edrive --home /home/edrive --disabled-password --gecos ''
	sudo usermod -a -G adm,cdrom,sudo,dip,plugdev,lpadmin,sambashare edrive
fi

apt-get update -qq

apt-get install -yy software-properties-common
add-apt-repository -yy ppa:webupd8team/java

echo "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main" >> /etc/apt/sources.list
echo "deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe" >> /etc/apt/sources.list

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




chown -R root:root /home/edrive/Edrive
chmod -R 755 /home/edrive/Edrive


# Install SOEM libraries
# ============================================== #

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

# Install E-Scope libraries
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

cd $ESCOPE_DIR/fungen/
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

# Install gcc-4.9 and g++-4.9
# ============================================== #
apt install -yy gcc-4.9
apt install -yy g++-4.9

rm /usr/bin/gcc
rm /usr/bin/g++

ln -sf /usr/bin/gcc-4.9 /usr/bin/gcc
ln -sf /usr/bin/g++-4.9 /usr/bin/g++



