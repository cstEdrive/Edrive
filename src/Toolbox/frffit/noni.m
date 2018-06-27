function ni=noni(i,n)
% NONI   ni=noni(i,n)
% finds all integers from 1 to n not equal to any integer in vector i
% and makes a column vector of these.
% Example
%        >> noni([1 1 3],5)
%             2
%             4
%             5
%
% This function is useful for deleting repeated integers in a vector:
%          i = noni(noni(i,max(i)),max(i))        (i is also sorted)
%
% If n is a vector with length>1 then noni returns the elements of i
% that do not occur in vector n. To force this interpretation enter
% [n(1) n] as second input argument.

% file           : noni.m
% author         : P.Wortelboer
% used functions :
% last change    : 19 maart 1997
% version        : 1e

if isempty(i)
  if length(n)==1
    ni=[1:n]';
  else
    ni=n(:);
  end
  return
end

if length(n)==1
  ni=[0:n]';
  ni([1;i(:)+1])=[];
elseif isempty(n)
  ni=[];
else
  if n(1)==n(2)
    n(1)=[];
  end
  i=i(:)';
  ni=n(:);
  [ii,jj]=find(ni(:,ones(1,length(i)))==i(ones(length(ni),1),:));
  ni(ii)=[];
end

% Copyright Philips Research Labs, 1998,  All Rights Reserved
