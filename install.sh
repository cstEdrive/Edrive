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
add-apt-repository -yy ppa:webupd8team/java
add-apt-repository -yy ppa:jonathonf/gcc-6.3

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

# Install gcc-6.3 and g++-6.3
# ============================================== #

cd /home/ebox/Downloads/

wget https://ftp.nluug.nl/pub/gnu/gcc/gcc-6.3.0/gcc-6.3.0.tar.gz
# wget ftp://ftp.nluug.nl/mirror/languages/gcc/releases/gcc-6.3.0/gcc-6.3.0.tar.gz
tar xf gcc-6.3.0.tar.gz
rm gcc-6.3.0.tar.gz*
cd gcc-6.3.0
contrib/download_prerequisites
cd ..
mkdir build && cd build
../gcc-6.3.0/configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=/usr/local/gcc-6.3 --enable-checking=release --enable-languages=c,c++,fortran --disable-multilib --program-suffix=-6.3
make -j 8
sudo make install

# sudo apt-get install -yy gcc-5 g++-5
# cd /usr/bin
# rm gcc
# rm g++
# ln -sf /usr/bin/gcc-5 /usr/bin/gcc
# ln -sf /usr/bin/g++-5 /usr/bin/g++


# cd /home/ebox/Downloads
# wget https://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-6.3.0.tar.bz2
# tar jxvf gcc-6.3.0.tar.bz2
# cd gcc-6.3.0
# ./contrib/download_prerequisites
# cd ..
# mkdir gcc-build && cd gcc-build
# ../gcc-6.3.0/configure -v --prefix=$HOME/gcc-6.3.0 --disable-multilib

# make
# sudo make install


echo "Modifications to be made manually"

echo "1. Within /home/ebox/svn/src/E-box/Targets/ectarget/ec_main.c"

echo "   change line 560 to:"

echo "   /usr/local/MATLAB/R2015b/rtw/c/ectarget/ectarget.u1"



