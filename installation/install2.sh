
















apt-get install sysvbanner -y




banner "EBOX package installation script"

echo "--------------------------------"

echo

sleep 3




echo

echo "Update repositories"

sleep 2

apt-get update




#echo

#echo "make /software directory"

#sleep 2

#mkdir /software




#echo

#echo "install nfs-common"

#sleep 2

#apt-get install rpcbind nfs-common -y




#echo

#echo "install network-manager-pptp"

#sleep 2

#apt-get install network-manager-pptp -y







#echo

#echo "mount the TU/e server"

#sleep 2

#mount hercules.tue.nl:/vol/hercules/software/linux /software




echo

echo "Install libgtk2.0-dev"

sleep 2

apt-get install libgtk2.0-dev -y

echo "Removing GCC & G++ 4.9 ..."

sleep 1

apt-get purge gcc-4.9 -y

apt-get purge g++-4.9 -y




echo

echo "install GCC & G++ 4.7"

sleep 2

add-apt-repository ppa:ubuntu-toolchain-r/test

apt-get update

apt-get install gcc-4.4 -y 

apt-get install g++-4.4 -y

cd /usr/bin

ln -sf gcc-4.4 gcc

ln -sf g++-4.4 g++


. Zxzaa.

echo

echo "install java"

sleep 2

add-apt-repository ppa:webupd8team/java

apt-get update

apt-get install oracle-java6-installer -y




echo

echo "Install Gedit"

sleep 2

apt-get install gedit -y




echo

echo "Install SSH"

sleep 2

apt-get install ssh -y




echo

echo "Install Make"

sleep 2

apt-get install make -y




echo

echo "Install -lpq"

sleep 2

apt-get install libpq-dev -y




echo

echo "Install ethtool"

sleep 2

apt-get install ethtool -y




echo

echo "Install postgresql & postgresql-client"

sleep 2

apt-get install postgresql postgresql-client -y




echo

echo




echo

echo "Changing ownership and permissions"

chown -R root:root /home/ebox/svn

chmod -R 777 /home/ebox/svn




echo

banner "Matlab Installation"

sleep 1

/software/matlab-2014b/install




banner "INSTALLATION COMPLETE"




echo "Modifications to be made manually"

echo "1. Within /home/ebox/svn/trunk/src/E-box/Targets/ectarget/ec_main.c"

echo "   change line 560 to:"

echo "   /usr/local/MATLAB/R2014b/rtw/c/ectarget/ectarget.u1"



