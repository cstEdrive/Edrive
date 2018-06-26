function [Y,w,l]=simlsort(X,v,iord)
% SIMLSORT  [Y,w,l] = simlsort(X,v,iord) 
% SIMuLtaneous SORT of X and v with reordering governed by v.
% Let l define the indices of the ordering then
%     w=v(l)
%     Y=X(:,l)  for v row vector
%     Y=X(l,:)  for v column vector
% iord: increasing (default) or decreasing order
%       'in', 'de'

% file           : simlsort.m
% last change    : 2 december 1991
% author         : P.Wortelboer
% used functions :

nv=size(v);
nX=size(X);
if nargin==2
  iord='in';
end
if nv(1)==nX(1) & nv(2)<=1
  if iord=='in'
    [w,l]=sort(v);
  else
    [w,l]=sort(-v);
    w=-v;
  end
  Y=X(l,:);
elseif nv(2)==nX(2) & nv(1)==1
  if iord=='in'
    [w,l]=sort(v);
  else
    [w,l]=sort(-v);
    w=-v;
  end
  Y=X(:,l);
else
  help simlsort;
  error('v not of same size as row or column of X');
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
