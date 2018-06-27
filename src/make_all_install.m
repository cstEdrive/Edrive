% make_all_install
% makefile which installs a complete package for EtherCAT and E/box
% if desired the specific toolboxes and files for TU/e can be installed

%% Change Log
% 07 February 2015: symbolic link in /usr/bin for geteth.o [MPlantinga]

current_ebox_dir = cd;

% Build libraries independent of Matlab
!./buildall

% Build E-Scope
cd Apps/E-Scope/
!./buildescope
addpath([pwd,'/engine'])
addpath([pwd,'/guiv25'])
addpath([pwd,'/tde_gate'])

cd engine
make
cd ..

cd tde_gate
make

cd(current_ebox_dir)

versioninfo = ver('matlab');

%% >>>> MODIFICATION: Added Matlab 2013b
% If version is not R2007b or R2010a or R2011b or R2013b
if ~strcmp(versioninfo.Release,'(R2007b)') && ~strcmp(versioninfo.Release,'(R2010a)') && ~strcmp(versioninfo.Release,'(R2011b)') && ~strcmp(versioninfo.Release,'(R2013b)')
    disp('Warning: Unsupported version of MATLAB')
end

% Install Toolboxes
cd Toolbox
make
cd(current_ebox_dir)
    
% Install target
cd Targets
make;
cd(current_ebox_dir)


%% >>>> NOTE: Matlab 2013b is installed in /usr/local/MATLAB/R2013b -> CHECK PATH line 52
% Create symbolic links in matlabroot only
cd(matlabroot)
cd rtw/c/ectarget/
eval(['!ln -sf ',current_ebox_dir,'/Libs/Coin/u1.h u1.h'])
eval(['!ln -sf ',current_ebox_dir,'/Libs/ec/ec.h ec.h'])
eval(['!ln -sf ',current_ebox_dir,'/Libs/Timer_posix_AK/timer.h timer.h'])
if ~strcmp(matlabroot,'/usr/local/matlab2011b')
    disp('Matlab location is different from default')
    eval(['!sed ''s/\/usr\/local\/matlab2011b/',strrep(matlabroot,'/','\/'),'/g'' ec_main.c > ec_main.temp'])
    !mv ec_main.temp ec_main.c
end
cd(current_ebox_dir)

% Add ectarget directory to path
eval( ['addpath ' matlabroot '/rtw/c/ectarget/']);
eval( ['addpath ' matlabroot '/rtw/c/ectarget/sfunctions']);
savepath
rehash

clear current_ebox_dir versioninfo install_tue

% Creade symbolic link in /usr/bin for geteth.o
system('ln -s /home/ebox/svn/trunk/src/E-box/Scripts/geteth /usr/bin/geteth');