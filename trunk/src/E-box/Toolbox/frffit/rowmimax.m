function [y,l] = rowmimax(X)
% ROWMIMAX  [y,l] = rowmimax(X)
% searches the minimum over the maxima of the rows of a matrix X
% y : the rowminmax value
% l : the entry of X containing y 

% file           : rowmimax.m
% last change    : 27 juli 1993
% author         : P.Wortelboer
% used functions :

X=X';
[nr,nc]=size(X);
if nr>1,
  [X,L]=max(X);
else,
  L=ones(1,nc);
end;
[y,l(1)]=min(X);
l(2)=L(l(1));

% Copyright Philips Research Labs, 1994,  All Rights Reserved
