function [a,b,c,d] = frfit(fr,hz,struc,WF)
%FRFIT   [a,b,c,d] = frfit(fr,hz,struc,WF), or
%        [num,den] = frfit(fr,hz,struc,WF)
%  output-error-LS fit of siso model [a,b,c,d] to continuous-time and 
%  discrete-time frequency response data fr 
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

% file           : ext4/frfit.m
% author         : M. Steinbuch, P Wortelboer
% used functions : ext4/...
%		   frqls1
%		   Wor/...
%                  mvfr2vmu unpek
% last change    : 31 juli 1998
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
G = frqls2(Z,struc,WF);

[a,b,c,d]=unpek(G);
if nargout==2,
  [a,b]=ss2tf(a,b,c,d,1);
end
  
% Copyright Philips CFT , 1998,  All Rights Reserved
