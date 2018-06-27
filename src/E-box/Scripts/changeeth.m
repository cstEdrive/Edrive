% % Filename: changeeth.m
%
% Abstract: Script to change eth settings for EtherCAT
%
% Usage: changeeth(1) to use eth1 etc
%
% Michael Ronde 
% (2012-01-11) 

% Changes:
% 20120111 - Intitial version 
% 20120111 - Added check on file existence
% 20150202 - Read geteth.c port and add it automatically to variable netif.
%            The port no. doesn't have to be added manually
% ________________________________ 
% Eindhoven University of Technology 
% Dept. of Mechanical Engineering 
% Control Systems Technology group 
% PO Box 513, WH -1.127 
% 5600 MB Eindhoven, NL 
% T +31 (0)40 247 2798 
% F +31 (0)40 246 1418 
% E m.j.c.ronde@tue.nl 

function changeeth(netif)

if nargin == 0
    netif=system('geteth &');
    fprintf('EBOX is connected to port eth%d\n',netif)
end

if ~isnumeric(netif)
    disp('netif must be numeric')
    return
end

cd /home/ebox/svn/trunk/src/E-box/Scripts/
% cd /home/ebox/Documents/SVN/Scripts/

save('eth_defaults','netif')

cd /home/ebox/svn/trunk/src/E-box/Templates/Generic/
% cd /home/ebox/Documents/SVN/Templates/Generic/
if (exist('ebox.mdl','file') == 4)
    ectarget_settings('ebox',netif)
end
cd /home/ebox/svn/trunk/src/E-box/Templates/Specific/pato/
% cd /home/ebox/Documents/SVN/Templates/Specific/pato/
if (exist('pato01.mdl','file') == 4 )
    ectarget_settings('pato01',netif)
end

cd /home/ebox/svn/trunk/src/E-box/Scripts/
% cd /home/ebox/Documents/SVN/Scripts

eval(['!sudo ethtool -C eth',num2str(netif),' rx-usecs 0 rx-frames 1 tx-usecs 0 tx-frames 1'])

