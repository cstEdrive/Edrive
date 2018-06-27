% Test delay of E-box in adc-dac loopback

fs      =   4096;
tend    =   100;

% Derived parameters
ts      =   1/fs;
NFFT    =   tend*fs/100;

%
if(1)
    rtwbuild('test_delay')
    !./test_delay
end

%%
load('test_delay')

[P11,f11] = tfestimate(rt_dac0(:,2),rt_adc0(:,2),hanning(NFFT),0.5*NFFT,NFFT,fs);
[P22,f22] = tfestimate(rt_dac1(:,2),rt_adc1(:,2),hanning(NFFT),0.5*NFFT,NFFT,fs);

P11 = frd(P11,f11,'Units','Hz');
P22 = frd(P22,f22,'Units','Hz');

% Model Zero-Order Hold effect
ZOH     =   (1-exp(-i*2*pi*f11*ts))./(i*2*pi*f11*ts);
sys_ZOH =   frd(ZOH,f11,'Units','Hz');

% Model computation time
tc=1.5*ts;
CT=exp(-i*2*pi*f11*tc);
sys_CT=frd(CT,f11,'Units','Hz');

% Extract mechanical part of system
sys_ZOH_and_CT=sys_CT*sys_ZOH;

figure
bode(P11,P22,sys_ZOH_and_CT)
% print('-dpng','delay_dac_adc_loopback')
