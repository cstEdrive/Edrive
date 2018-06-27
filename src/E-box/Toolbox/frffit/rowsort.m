function [Y,I]=rowsort(X)
% ROWSORT  [Y,I] = rowsort(X)
%          sorts each row in X separately in ascending order.
%          X : matrix
%          Y : matrix with sorted rows
%          I : matrix with sorting indices per row
% For xk,yk,ik the k'th row of X,Y,I the following holds:  yk=xk(ik)
% example [Y,I]=ROWSORT([0 4 2;5 3 1])
%          Y   = [0 2 4;1 3 5]
%            I = [1 3 2;3 2 1]

% file           : rowsort.m
% last change    : 13 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
Y=zeros(nr,nc);
I=zeros(nr,nc);
for i=1:nr
  [Y(i,:),I(i,:)]=sort(X(i,:));
end
% Copyright Philips Research Labs, 1994,  All Rights Reserved
