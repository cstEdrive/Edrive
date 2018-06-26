function y=rowprod(X)
% ROWPROD  y = rowprod(X)
%          calculates product for each row in X separately.
%          X : matrix
%          y : column vector with products per row

% file           : rowprod.m
% last change    : 10 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[ones(nr,1) X];
y=prod(X')';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
