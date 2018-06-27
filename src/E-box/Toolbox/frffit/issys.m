function   i012=issys(G)
% ISSYS    i012=issys(G)
% Checks if G is or contains SYSTEMS:
% Returns	1	for CONTINUOUS-TIME SYSTEM
%		1+j	for DISCRETE-TIME SYSTEM
%		1-j	for consistent DISCRETE-TIME SYSTEM
%		n	for set of SYSTEMs
%               's'     for a SYSTEM structure
% and otherwise 0.

% file           : issys.m
% author         : P.Wortelboer
% used functions :
% last change    : 3 november 1997
% version        : 1e

[nr,nc]=size(G);
if nr==0 | nc==0
  i012=0;
  return
end
[ir,ic]=find(real(G)==-Inf);
ns=length(ir);
if ns>1                 % multiple SYSTEMS
  i012=ns;
  return
elseif ns==1 
  if ir~=nr | ic~=nc
    % SYSTEM structure
    i012='s';
    return
  end
elseif ns==0
  i012=0;
  return
end
n=G(ir(1),ic(1));
if real(n)==-Inf	% SYSTEM
  T=imag(n);
  if T>0
    i012=1+j;
  elseif T<0
    i012=1-j;
  else
    i012=1;
  end
else
  i012=0;
end

% Copyright Philips Research Labs, 1996,  All Rights Reserved
