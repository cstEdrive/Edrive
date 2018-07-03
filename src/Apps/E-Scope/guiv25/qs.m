function qs(varargin)

% QS  QadScope experimentation software for use with TUeDACS/

% QS  QadScope experimentation software for use with TUeDACS/1 QAD/AQI modules
%
% QS starts measuring with the default sampling rate of 4096 Hz.
%
% QS(MAXFREQ) starts with a sampling rate of MAXFREQ Hz.
%
% QS(MAXFREQ,DAC_NDOWN) starts with a sampling rate of MAXFREQ Hz and a
% down-sampled D/A-rate of MAXFREQ/(1+DAC_NDOWN) Hz.

%% LOG: 
%17 February 2015 [MPlantinga]: added ethernet port renaming script
%                               eth_check.sh
%01 March 2015 [MPlantinga]: added script to check if qs engine is in
%                            simulation mode. If this is the case change 
%                            it to experiment (EBOX) mode


%% Check if engine is in simulation mode, if so reset to experiment mode
sim_ind='/home/ebox/svn/trunk/src/E-box/Apps/sim_exp/sim_indicator';

% chech if the simulation indicator file (sim_indicator) is present to see
% if the engine has to be recompiled to experiment mode
if ~exist(sim_ind,'file')==1
    %run compilation script
    system('/usr/local/simexp/experiment/mlsim >/dev/null 2>&1');
end


%% Check if eth port is named eth0, eth1 or eth2, otherwise rename to eth0
%   reboot is mandatory if changes are made.

eth_port_check=system('/home/ebox/svn/trunk/src/E-box/eth_port/eth_check.sh');


% if changes are made warn that reboot is mandatory & quit
if eth_port_check==1
    %recompile experiment
    system('/usr/local/simexp/experiment/mlsim >/dev/null 2>&1');
    fprintf('YOU NEED TO RESTART YOUR COMPUTER NOW IN ORDER TO CONTINUE!\n')
    error('PLEASE RESTART YOUR COMPUTER NOW! If you do not restart your computer, you will not be able to access the EBOX')
end


%%
clear mex;

maxfreq=4096;
frame_length = maxfreq;
dac_ndown=0;
netif=0;

% Override defaults by mat file created by changeeth for changing default
% netif
if (exist('eth_defaults.mat','file')==2)
    load eth_defaults.mat
end

if nargin>0
    maxfreq=varargin{1,1};
end
if nargin>1
    frame_length=varargin{1,2};
end
if nargin>2
    dac_ndown=varargin{1,3};
end
if nargin>3
    netif=varargin{1,4};
end

netif = ['eth',num2str(netif)];

%cmd=sprintf('!rxvt -title "QadScope engine" -e preempt_run_usb /usr/local/matlabr14sp2/toolbox/qadscope/qseng_usb_preempt %f %d %d &',maxfreq,dac_ndown,ical);
% cmd=sprintf('!"E-Scope engine" -e "taskset 02 /home/ebox/svn/trunk/src/E-box/Apps/E-Scope/engine/eseng %d %d %d %s " &',maxfreq,frame_length,dac_ndown,netif);
% cmd=sprintf('!gnome-terminal -t "E-Scope engine" -e "taskset 02 /home/ebox/svn/trunk/src/E-box/Apps/E-Scope/engine/eseng" &');
% eseng_path=sprintf('!gnome-terminal -t "/home/ebox/svn/trunk/src/E-box/Apps/E-Scope/engine/eseng"');
% eval(cmd)

% eval(cmd);

% system('/home/ebox/svn/trunk/src/E-box/Apps/E-Scope/engine/eseng &');

eth_port = system('geteth');
[status, geteth_output] = system('geteth')
eth_port = regexp(geteth_output, 'Ethercat stack connected to ', 'split')
eth_port = eth_port{2}(1:end-1)

geteth_path = sprintf('/home/ebox/Edrive/src/E-box/Apps/E-Scope/engine/eseng %d %d %d eth%d &',maxfreq, frame_length, dac_ndown, eth_port);
system(geteth_path);

% wait some time
pause(5);

% start gui
qadscope

pause(5)
