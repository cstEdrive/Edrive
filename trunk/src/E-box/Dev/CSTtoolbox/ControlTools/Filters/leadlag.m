function C = leadlag(f1,f2)
%LEADLAG continuous lead lag filter
%   usage: leadlag(f1,f2)
%   corner freqs: f1 and f2 in Hz
s=tf('s');
C=[(1+s/(f1*2*pi))/(1+s/(f2*2*pi))];
end

