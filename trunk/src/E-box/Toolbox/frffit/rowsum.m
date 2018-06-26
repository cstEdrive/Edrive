function y=rowsum(X)
% ROWSUM  y = rowsum(X)
%           calculates sum for each row in X separately.
%           X : matrix
%           y : column vector with sums per row

% file           : rowsum.m
% last change    : 10 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[zeros(nr,1) X];
y=sum(X')';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
