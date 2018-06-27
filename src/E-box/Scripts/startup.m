% startup.m


% Override defaults by mat file created by changeeth for changing default
% netif
if (exist('eth_defaults.mat','file')==2)
    load eth_defaults.mat
    eval(['!sudo ethtool -C eth',num2str(netif),' rx-usecs 0 rx-frames 1 tx-usecs 0 tx-frames 1'])
    clear netif
else
    disp('Default eth port not found run changeeth(x), where x is the ethernet port number')
end