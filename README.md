# Edrive
This repository installs the necessary software and dependencies for the E-Drive.

## Requirements
Installation has been tested on:
- Lubuntu 18.04
- Matlab 2017b

Note that different versions of Matlab require different versions of gcc and g++. The compiler that is used for Matlab 2018a is gcc-4.9 and g++-4.9 Check the following site if you choose a different version of Matlab: https://nl.mathworks.com/support/sysreq/previous_releases.html


# 1. Create a Lubuntu USB drive
The following sections describes the steps to be followed.

### 1.1 Create Live USB drive
For this section, you need **1 USB drive** with at least **4GB** of memory. We will call this the *Live USB*.

1. Download and install Unetbootin from [https://unetbootin.github.io/]
2. Download the 64bit version of Lubuntu from [https://lubuntu.net/downloads/]
3. Connect the USB to be used as Live USB to the computer
4. Open Unetbootin
5. Select `Diskimage` and enter the path to the downloaded Lubuntu `*.iso` file (step 2)
6. Under type select `USB Drive` and subsequently the correct USB drive.
7. **IMPORTANT:** make sure the drive you selected is the correct USB drive, writing files to an incorrect USB drive or Hard Drive cannot be undone. Use applications, such as `Disk Utility` (Mac), `gparted` (Linux) or `EaseUS Partition Master` (Windows), to verify that you have selected the correct drive.
8. If everything has been entered correctly in the previous steps, click `OK` to create the Live USB. This takes a couple of minutes.

### 1.2 Install Lubuntu on USB drive
For this section, you need 2 USB drives:
1. The Live USB of Section 1
2. A USB drive with at least 16 GB of memory. We will call this the *Lubuntu USB*.
For Macintosh computers, you will have to do some additional changes to the bootloader. These changes are explained in Section 3.

Install as follows:
1. Boot the Live USB drive created in the previous section. For this you might have to change your BIOS settings to allow booting from a USB drive. If you don't know how to boot from a USB drive, look this up on the Internet for your computer.
2. Once booted, connect the USB drive where you want to install the Lubuntu on, i.e. the Lubuntu USB.
3. Run `gparted` by typing 
    ```
    sudo su
    gparted
    ```
    in a terminal.
4. Select the correct drive and unmount USB Lubuntu in gparted: `Partition > Unmount`
5. Create a new partition table: `Device > Create Partition Table ...` and select `msdos`
6. Create two partitions: `Partition > New`
    - primary `unformatted` partition of <usb_size> - 1024 MiB
    - primary `linux-swap` partition of 1024 MiB
7. Apply changes: `Edit > Apply All Operations` and exit gparted.
8. Double click: `Install Lubuntu 16.04.3 LTS`
9. In the installation window, under `Installation type`, select `Something else` and continue.
10. Double click the first partition of the disk where you want to install Lubuntu, e.g. `/dev/sdc1`. In the popup window, select `Ext4 Journaling file system`, click the checkbox `Format the partition` and type `/` under `Mount point:`.
11. Make sure the partition is selected when you go back to the original window, i.e. the line `/dev/sdc1 ext4 / ...`.
12. Under `Device for boot loader installation:`, select the drive where you are going to install Lubuntu, e.g. `/dev/sdc` (NOT the partition, but the entire drive). Make sure you don't make any mistakes in this window, as this can destroy the OS and files on your computer.
13. Click `Install Now` and make sure in the popup window everything is correct.
14. In the `Who are you?` window, set the following values:
    ```
    Your name: `edrive`
    Your computer's name: `edrive`
    Pick a username: `edrive`
    Choose a password: `<your password>`
    Confirm your password: `<your password>`
    Select `Log in automatically`
    ```
15. Click `Continue` and let the installation do its work.

### 1.3 Macintosh Installation
To be able to boot the Edrive from a Mac, the EFI partition has to be in HFS+ file format. For this section, you need 2 USB drives:
1. The Live USB of Section 1
2. The Lubuntu USB of Section 2.

Proceed as follows:

1. To avoid any possible problems, make sure the Lubuntu USB you created in Section 2 has been created on a Mac. Installations from a PC might work too.
2. Connect **both** the Live USB created in Section 1 as the Lubuntu USB drive created in Section 2 to the Mac and boot the computer while holding down the Option key.
3. In the boot menu of Mac, select the Live USB and press Enter.
4. In the following Grub menu, where you get the option to `Try Lubuntu without installing`, press `c` to enter the Grub console.
5. List all drives by typing `ls`.
6. Determine which drive corresponds to the Lubuntu USB drive. This can be best done by looking if the *Home* directory of the drive is `/home/edrive`. To check that for drive `hdX`, with `X` being an integer corresponding to one of the drives, type 
    ```
    ls (hdX,msdosY)/home
    ```
    where `Y` is an integer corresponding to partition Y of drive X. Note that if you have created the Lubuntu USB as in Section 2, there will only be two partitions, i.e. `msdos1` and `msdos2`. If one of the partitions of disk `X` prompts `edrive`, this is the Lubuntu drive.
7. For the drive found in the above step, check which partition contains the kernel by typing `ls (hdX,msdosY)/boot/grub` to see if it prompts the kernel.
8. Set the found drive `X` and partition `Y` as root by typing:
    ```
    set root=(hdX,msdosY)
    ```
9. Print the UUID of the drive by typing  
    ```
    ls -l (hdX,msdosY)
    ```
10. Tell Grub where Linux is stored by typing:
    ```
    grub> linux /boot/vmlinuz<TAB>.efi.signed root=UUID=<UUID>
    ```
    where `<TAB>` means pressing the TAB key to autocomplete. Make sure that the `vmlinuz...` kernel ends with `.efi.signed`. Substitute `<UUID>` with the UUID that you printed in the previous step.
11. Set the Initial RAM Disk (initrd) by typing:
    ```
    initrd /boot/initrd<TAB>
    ```
12. Boot the Lubuntu USB by typing:
    ```
    boot
    ```
13. Once the system has booted, open a terminal and run the following commands to install packages:
    ```
    sudo add-apt-repository ppa:detly/mactel-utils
    sudo apt-get update
    sudo apt-get install mactel-boot hfsprogs gdisk grub-efi-amd64
    ```
14. Determine the name of the Lubuntu USB drive by typing:
    ```
    sudo fdisk -l
    ```
15. Run `gdisk` to create a new partition table and give the disk found in the previous step as argument, e.g.
    ```
    sudo gdisk /dev/sdX
    ```
    where `X` is the letter that corresponds with the Lubuntu USB drive.
16. In the `gdisk` interactive menu, type `n` to create a new partition and set it up as with the defaults for `Partition number`, `First sector` and `Last sector`.
17. Choose `AF00` for Hex code or GUID to create an Apple HFS/HFS+ partition.
18. Write the settings to disk by typing `w`.
19. Rescan all disks for new partitions by typing
    ```
    partprobe
    ```
19. Create new filesystem for the created partition by typing
    ```
    sudo mkfs.hfsplus /dev/sdXY -v Ubuntu
    ```
    where `X` is the letter that corresponds to the Lubuntu USB and `Y` is the integer that corresponds to the newly created partition, e.g. `3`.
20. Update the `fstab`:
    ```
    sudoedit /etc/fstab
    ```
    by deleting the lines that define to `/boot/efi`, e.g.
    ```
    # /boot/efi was on /dev/sda1 during installation
    UUID=xxxx  /boot/efi       vfat    defaults        0       1
    ```
21. Unmount anything mounted on `/boot/efi`. Check if any disk is mounted by typing `mount | grep /boot/efi` and if this prompts a disk, e.g. `/dev/sda1 on /boot/efi`, unmount it by typing:
    ```
    sudo umount /dev/sdXY
    ```
    where `X` and `Y` are the letter and the partition of the disk mounted on `/boot/efi`
22. Run the following commands:
    ```
    sudo bash -c 'echo UUID=$(blkid -o value -s UUID /dev/sdXY) /boot/efi auto defaults 0 0 >> /etc/fstab'
    ```
    where `X` and `Y` have to be substituted by the letter of the drive and the integer of the partition that correspond to the new partition of the Lubuntu USB.
23. Reinstall Grub by typing:
    ```
    sudo mkdir -p "/boot/efi/EFI/$(lsb_release -ds)/"
    sudo bash -c 'echo "This file is required for booting" > "/boot/efi/EFI/$(lsb_release -ds)/mach_kernel"'
    sudo bash -c 'echo "This file is required for booting" > /boot/efi/mach_kernel'
    sudo grub-install --target x86_64-efi --boot-directory=/boot --efi-directory=/boot/efi --bootloader-id="$(lsb_release -ds)"
    sudo hfs-bless "/boot/efi/EFI/$(lsb_release -ds)/System/Library/CoreServices/boot.efi"
    sudo sed -i 's/GRUB_HIDDEN/#GRUB_HIDDEN/g' /etc/default/grub
    sudo sed -i 's/GRUB_TIMEOUT=10/GRUB_TIMEOUT=0.1/' /etc/default/grub
    sudo grub-mkconfig -o /boot/grub/grub.cfg
    ```
24. Fix the default bootloader by booting Mac OS and then under `System Settings > Startup Disk` select the Mac OSX system.

# 2. Install Ethercat Software
1. Install Git:
    ```
    sudo su
    apt-get install git
    ```
2. Clone the repository:
    ```
    cd /home/edrive
    git clone https://github.com/cstEdrive/ebox.git
    ```
3. Run the installation script
    ```
    cd /home/edrive/ebox
    ./install.sh
    ```

# 3. Install Matlab
1. Go to `www.mathworks.com` and download the **Matlab 2017b** installer: [https://www.mathworks.com/downloads/web_downloads/download_release?release=R2017b]. Other versions are possible too, however the `gcc` and `g++` versions have to be modified accordingly. See also the installation script of the previous section `install.sh`. 
2. Go to the location where you extracted the installer and install Matlab by typing 
    ```
    ./install
    ```
3. Log in with a Mathworks Account.
4. Accept terms of the license agreement and log in with your credentials.
5. Select the license and keep the installation folder unmodified as `/usr/local/MATLAB/R2017b`.
3. Install only the following packages:
    - Matlab 9.3
    - Simulink 9.0
    - Matlab Coder 3.4
    - Simulink Coder 8.13
    - Control System Toolbox (used for Shapeit)
4. When registering Matlab, make sure to enter `root` as username.

# 4. Additional Settings
1. Remove `System Program Problem Detected` popups. These popups allow you to send a crash report to the developers. Whenever a program crashes, a crash report is created and stored under `/var/crash/*.crash`. To remove the popup from appearing when a crash occurs, edit the `/etc/default/apport` file by setting modifying the `enabled` line as follows:
    ```
    enabled=0
    ```
2. Change the copy-pasting shortcut keys in Matlab. Go to `Preferences > Keyboard > Shortcuts` and select `Windows Default Set` under `Active settings`.

# 5. Errors
1. `Failed to load module: /usr/lib/x864_64 linux gnu/gio/modules/libgiolibproxy.so`
    This error occurs when the version of Java JRE that ships with MATLAB loads a library outside of MATLAB which in turn is incompatible with the version of `libstdc++.so.6` that ships with MATLAB.To resolve the issue, navigate to MATLAB R2017b installation location, i.e. `/usr/local/MATLAB`, and remove the `libstdc++.so.6` library file shipped with MATLAB R2017b, so that MATLAB will use the system version of that library:


