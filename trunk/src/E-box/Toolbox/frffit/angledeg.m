function [ph]=angledeg(Freq,wrapopt)
% [ph]=angledeg(Freq,wrapopt)
%
% determination of phase in degrees of complex frequency response data Freq
% (Freq may be any complex matrix)
%
% optional: wrapopt = 0 --> phase remaines wrapped between -180 and 180 deg.
%                   = 1 --> phase is smoothed with function ADDTWOPI
%

% file           : ANGLEDEG.M           ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 30 aug 1990
% final update   : 30 aug 1990
% used functions : addtwopi, phase

if nargin==1,wrapopt=1;end
if nargin>2,error('too many input arguments'),end

ph=angle(Freq);
if wrapopt==1, ph=addtwopi(ph);end
ph=ph*180/pi;

return;



