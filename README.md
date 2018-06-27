# Edrive
This repository installs the necessary software and dependencies for the E-Drive.

## Requirements
Installation has been tested on:
- Ubuntu 18.04
- Lubuntu 18.04

with Matlab 2018a

Note that different versions of Matlab require different versions of gcc and g++. The compiler that is used for Matlab 2018a is gcc-6.3.0 and g++-6.3.0. Check the following site if you choose a different version of Matlab: https://nl.mathworks.com/support/sysreq/previous_releases.html
## Installation
Make sure to be super user for the installation.
1. Install git
    ```
    sudo su
    apt-get install git
    ```
2. Clone this repository
    ```
    git clone https://github.com/cstEdrive/Edrive.git
    ```
3. Run the installation script
    ```
    cd Edrive
    ./install.sh
    ```

