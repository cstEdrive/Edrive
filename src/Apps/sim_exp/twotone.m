%% Two Tone Simulation Initialization Script

%% Define strings
sim_ind='touch /home/ebox/svn/trunk/src/E-box/Apps/sim_exp/sim_indicator';
make_sim_ind='touch /home/ebox/svn/trunk/src/E-box/Apps/sim_exp/sim_indicator';
rm_sim_ind='rm /home/ebox/svn/trunk/src/E-box/Apps/sim_exp/sim_indicator';

%% Execute script
% make simulation indicator file sim_indicator
system(make_sim_ind);

%run compilation script
system('/usr/local/simexp/twotone/mlsim >/dev/null 2>&1');

pause(1)
%start qadscope
qs

pause(1)

% remove simulation indicator file sim_indicator
system(rm_sim_ind);
clear;

