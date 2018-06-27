function [a,b,c,d] = frsfit(fr,hz,struc,WF)
%FRFIT   [a,b,c,d] = frsfit(fr,hz,struc,WF), or
%        [num,den] = frsfit(fr,hz,struc,WF)
%  output-error-LS stable fit of siso model [a,b,c,d] to continuous-time and 
%  discrete-time frequency response data fr (for possible unstable fits take frfit.m)
%  
%  < fr      : frequency response data
%  < hz      : hz = vector of frequencies [Hz](if negative --> discrete case)
%  < struc   : [nden,nnum,nint]
%              nden,nnum  = order of denominator,numerator (def: nnum=nden)
%              nint       = number of integrators
%  < WF      : optional weighting function of length(hz)  
%              ALSO: 0=uniform (default) 1=inv(abs(fr)) 2= as 1 for fr>1
%  > a,b,c,d : model, or
%  > num,den

% file           : ext4/frsfit.m
% author         : M. Steinbuch, P Wortelboer, J. Janssens
% used functions : ext4/...
%		   frqls1
%		   Wor/...
%                  mvfr2vmu unpek
% last change    : 7-3-2002
% version        : 1e+

if nargin<4
  WF=0;
  if nargin<3
    struc=[];
  end
end
w=2*pi*hz(:);
fr=fr(:);
Z=mvfr2vmu(fr,w);
G = frqls2s(Z,struc,WF);

[a,b,c,d]=unpek(G);
if nargout==2,
  [a,b]=ss2tf(a,b,c,d,1);
end
  
% Copyright Philips CFT , 1998,  All Rights Reserved
