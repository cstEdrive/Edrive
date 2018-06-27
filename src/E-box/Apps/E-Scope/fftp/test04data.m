close all
clear all
clc

!./buildlib
!./buildtest4

fs = 1024;
ts =  1/fs;

t = (0:ts:1-ts).';
u = randn(length(t),1);

beta = 5e-3;
wn = 2 * pi * 300;

G = tf(1,[1 2*beta*wn wn^2]);
Gd = c2d(G,ts,'zoh');



y = lsim(Gd,u,t);

tic
NFFT = 1024;
[Ghat,f] = tfestimate(u,y,hann(NFFT),0,NFFT,fs);
[C,fc] = mscohere(u,y,hann(NFFT),0,NFFT,fs);
toc
Ghat    =   frd(Ghat,f,'Units','Hz');
C       =   frd(C,fc,'Units','Hz');


figure
bode(Gd,Ghat)

save test04_input.txt -ascii -double u
save test04_output.txt -ascii -double y

%% Run
% !rm test04.txt
% !gnome-terminal -e "./test04 >> test04.txt"

%% Compare solutions

load test04_tfe.txt
j = sqrt(-1);
c = test04_tfe(:,2)+test04_tfe(:,3)*j;

load test04_coh.txt
coh = test04_coh(:,2);


figure
plot(db(abs(squeeze(Ghat.response))))
hold on
plot(db(abs(c)),'rx')


coh = frd(coh(1:513),fc,'Units','Hz');
figure
bodemag(C,coh)
