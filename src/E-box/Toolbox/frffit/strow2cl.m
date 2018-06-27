function strcol=strow2cl(strow,sep,fill)
% STROW2CL strcol = strow2cl(strow,sep,fill)
%        splits a row of characters separated by one or more
%        blanks in a matrix with each 'word' on a separate line
% 
% strow: line of characters
% sep  : separation character [default: ' ']
% fill : # filling character  [default: 32]
%      Example to show the difference between spaces [abs(' ')=32]
%      and trailing quasi-blanks with code 31.
% >> X=strow2cl('set.x.to.[1 0]','.',31);
% >> disp(X)
% set
% x
% to
% [1 0]
% >> disp(abs(X))
%  115   101   116    31    31
%  120    31    31    31    31
%  116   111    31    31    31
%   91    49    32    48    93


% file           : strow2cl.m
% author         : P.Wortelboer
% used functions : one4tupl
% last change    : 14 oktober 1997
% version        : 1e

if nargin<3
  fill=32;
  if nargin==1
    sep=' ';
  elseif length(sep)==0
    sep=' ';
  end
end
if length(strow)==0
  strcol=[];
  return
else
  strow=[strow sep];
  strow=one4tupl(strow,sep);
end
i=find(strow==sep);
if i(1)==1
  i(1)=[];
  i=i-1;
  strow(1)=[];
end
nstrow=length(strow);
ni=length(i);
id=diff([0 i])-ones(1,ni);
strcol=setstr(ones(ni,max(id))*fill);
for i=1:ni
  strcol(i,1:id(i))=strow(1:id(i));
  strow(1:id(i)+1)=[];
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
