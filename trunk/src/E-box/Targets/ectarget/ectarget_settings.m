function  ectarget_settings(filename,varargin)
% Filename: ectarget_settings.m
%
% Abstract: Sets the correct settings for simulink model to use with
% ectarget
%
% Usage: ectarget_settins(filename,netif,ts)
% 
% Inputs:
%   filename    -   filename of the simulink model
%   netif       -   number of the ethernet interface
%   ts          -   sample time, can either be numeric or string, i.e. variable name
%   ec_generic  -   flag, if 1 use generic port structure
%
% Examples:
% ectarget_settings('test',0,'ts')
% Sets all correct setting for a model named test with network interface
% eth0 and sample time ts (variable on workspace)
%
% ectarget_settings('test',1,1/1000)
% Sets all correct setting for a model named test with network interface
% eth1 and sample time 1/1000
%
% If option is blank [], default value is chosen, i.e. eth0 and ts=1/2048
%
% Michael Ronde 
% (2011-07-23) 

% Changes:
% 20110723 - Added option for generic port structure
% 20110705 - Initial version
% ________________________________ 
% Eindhoven University of Technology 
% Dept. of Mechanical Engineering 
% Control Systems Technology group 
% PO Box 513, WH -1.127 
% 5600 MB Eindhoven, NL 
% T +31 (0)40 247 2798 
% F +31 (0)40 246 1418 
% E m.j.c.ronde@tue.nl 

% Defaults:
netif       =   0;
ts          =   '1/2048';
ec_generic  =   0;

if nargin >= 2 && ~isempty(varargin{1})
    netif = varargin{1};
end
if nargin >= 3 && ~isempty(varargin{2})   
    ts = varargin{2};
end
if nargin >= 4 && ~isempty(varargin{3})   
    ec_generic = varargin{3};
end
open(filename)
cs = getActiveConfigSet(filename);

% Set the external mode
set_param(cs, 'SimulationMode', 'external')

% Select fixed-step solver with fixedstep
set_param(cs,'Solver','FixedStepDiscrete')
set_param(cs,'SolverName','FixedStepDiscrete')
set_param(cs,'SolverType','Fixed-step')
% ts can be char, i.e. variable name or numeric
if ischar(ts)
    set_param(cs,'FixedStep',ts)
else
    set_param(cs,'FixedStep',num2str(ts))
end
set_param(cs,'SolverMode','Auto')

% Select ectarget and make file
set_param(cs,'SystemTargetFile','ectarget.tlc')
% set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=0 -DEC_GENERIC"')
if ec_generic == 0
    switch(netif)
        case 0
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=0"')
        case 1
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=1"')
        case 2
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=2"')
        otherwise
            disp('Network interface > 2 not supported')
    end
else
    switch(netif)
        case 0
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=0 -DEC_GENERIC"')
        case 1
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=1 -DEC_GENERIC"')
        case 2
            set_param(cs,'MakeCommand','make_rtw OPTS="-DNETIF_ID=2 -DEC_GENERIC"')
        otherwise
            disp('Network interface > 2 not supported')
    end
end
set_param(cs,'TemplateMakefile','ec_unix.tmf')


% Enable external mode
set_param(cs,'ExtMode','on')
set_param(cs,'ExtModeMexFile','ext_comm')

% Save the model
save_system(filename,[],'OverwriteIfChangedOnDisk',true)
close_system(filename)
