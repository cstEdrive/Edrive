function Clp=lowpass(cutoff,blp);
%lowpass filter
s=tf('s');
wlp=cutoff*2*pi;
Clp=(1)/(s^2/wlp^2 + 2*blp*s/wlp +1);
end %function