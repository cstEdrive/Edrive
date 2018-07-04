% install_ebox.m
% makefile that installs a complete package for EtherCAT and E/box
% if desired the specific toolboxes and files for TU/e can be installed

current_ebox_dir = pwd;

% Build E-Scope
cd Apps/E-Scope/
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

% Install Toolboxes
cd Toolbox
make
cd(current_ebox_dir)
    
% Install target
cd Targets
make;
cd(current_ebox_dir)


% %% >>>> NOTE: Matlab 2013b is installed in /usr/local/MATLAB/R2013b -> CHECK PATH line 52
% % Create symbolic links in matlabroot only
% cd(matlabroot)
% cd rtw/c/ectarget/
% eval(['!ln -sf ',current_ebox_dir,'/Libs/Coin/u1.h u1.h'])
% eval(['!ln -sf ',current_ebox_dir,'/Libs/ec/ec.h ec.h'])
% eval(['!ln -sf ',current_ebox_dir,'/Libs/Timer_posix_AK/timer.h timer.h'])
% if ~strcmp(matlabroot,'/usr/local/matlab2011b')
%     disp('Matlab location is different from default')
%     eval(['!sed ''s/\/usr\/local\/matlab2011b/',strrep(matlabroot,'/','\/'),'/g'' ec_main.c > ec_main.temp'])
%     !mv ec_main.temp ec_main.c
% end
% cd(current_ebox_dir)

% % Add ectarget directory to path
% eval( ['addpath ' matlabroot '/rtw/c/ectarget/']);
% eval( ['addpath ' matlabroot '/rtw/c/ectarget/sfunctions']);
% savepath
% rehash

% clear current_ebox_dir versioninfo install_tue