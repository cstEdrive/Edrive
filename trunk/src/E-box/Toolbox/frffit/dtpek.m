function   [G,T]=dtpek(G,T)
% DTPEK      [G,T]=dtpek(G,T)
%
% Adds the sampling time T to a DISCRETE-TIME SYSTEM Gd:
% >> Gdt=dtpek(Gd,T)       % or >> Gdt=snsys(Gd,T)
% or extracts T from Gdt:
% >> [Gd,T]=dtpek(Gdt)
%
% dtpek should be used for the alternative CONSISTENT 
% DISCRETE-TIME SYSTEM  in the following way
% >> Gdtcon=dtpek(Gdcon,-T)
%    [Gdcon,minT]=dtpek(Gdtcon)
%
% See also: dsyscon, snsys, synfo, unpek 

% file           : dtpek.m
% last change    : 10 mei 1996
% author         : P.Wortelboer, E-mail wortel@prl.philips.nl
%                : Philips Research, Eindhoven, The Netherlands
% used functions : 

[nr,nc]=size(G);
if nr==0
  T=0;
  return
end
if nargin == 1
  I=G(nr,nc);
  if real(I)~=-Inf
    error('First input is not a SYSTEM MATRIX');
  end
  T=imag(I);
  G(nr,nc)=-Inf;
else
  G(nr,nc)=-Inf+sqrt(-1)*T;
end

% Copyright Philips Research Labs, 1996,  All Rights Reserved
