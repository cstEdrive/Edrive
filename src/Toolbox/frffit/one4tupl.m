function sr = one4tupl(s,t,rep)
% ONE4TUPL sr = one4tupl(s,t,rep)
% searches in a vector or string for occurences of target [t] and
% replaces n-tuples by a single [t] or even no [t] for [rep]==0.
%
% < s  : vector or string 
% < t  : target number or character      []->0 or ' '
% < rep: number of [t]s used for replacing matches [0|1],     []->1
% > sr : manipulated vector or string

% file           : one4tupl.m
% author         : P.Wortelboer
% used functions : 
% last change    : 14 oktober 1997
% version        : 1e

[nr,nc]=size(s);
if nr>1 & nc>1
  error('one4tupl only defined for vectors and strings, not for matrices')
end
if nargin<3
  rep=1;
  if nargin<2
    t=[];
    if nargin<1
      s=[];
    end
  end
end
if isempty(t)
  if isstr(s)
    t=' ';
  else
    t=0;
  end
elseif length(t)>1
  error('one4tupl only defined for scalar target or one-character target')
end
if isempty(s)
  sr=[];
else
  sr=s;
  i1=find(s==t);
  if rep==1
    i2=find(diff(i1)==1);
    sr(i1(i2))=[];
  elseif rep==0
    sr(i1)=[];
  else
    error(['rep=' int2str(rep) ' is not allowed'])
  end
end

% Copyright Philips CFT, 1997,  All Rights Reserved
