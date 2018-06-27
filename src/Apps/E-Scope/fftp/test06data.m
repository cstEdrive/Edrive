close all
clear all
clc

!./buildlib
!./buildtest6

!rm test06_input.txt
!rm test06_output.txt
!rm test06_tfe.txt

fs = 1024;
ts =  1/fs;

t = (0:ts:10-ts).';
u = randn(length(t),1);

beta = 5e-3;
wn = 2 * pi * 300;

G = tf(1,[1 2*beta*wn wn^2]);
Gd = c2d(G,ts,'zoh');



y = lsim(Gd,u,t);

tic
NFFT = 1024;
[Ghat,f] = tfestimate(u,y,hann(NFFT),0,NFFT,fs);
[Pxx,fxx] = pwelch(u,hann(NFFT),0,NFFT,fs);
[C,fc] = mscohere(u,y,hann(NFFT),0,NFFT,fs);
toc
Ghat    =   frd(Ghat,f,'Units','Hz');
C       =   frd(C,fc,'Units','Hz');
Pxx     =   frd(Pxx,fxx,'Units','Hz');


% figure
% bode(Gd,Ghat)

save test06_input.txt -ascii -double u
save test06_output.txt -ascii -double y

%% Run
% !rm test04.txt
% !gnome-terminal -e "./test04 >> test04.txt"

%% Compare solutions

load test06_tfe.txt
j = sqrt(-1);
c = test06_tfe(:,2)+test06_tfe(:,3)*j;

load test06_coh.txt
coh = test06_coh(:,2);

load test06_psdu.txt
psdu = test06_psdu(:,2);


figure
plot(db(abs(squeeze(Ghat.response))))
hold on
plot(db(abs(c)),'rx')


coh = frd(coh(1:513),fc,'Units','Hz');
figure
bodemag(C,coh)

psdu = frd(psdu(1:513),fc,'Units','Hz');
figure
bodemag(Pxx,psdu)


norm(squeeze(Ghat.response)-c(1:513,1),2)
norm(squeeze(C.response-coh.response),2)

temp = Pxx-psdu;
norm(squeeze(temp.response),2)

