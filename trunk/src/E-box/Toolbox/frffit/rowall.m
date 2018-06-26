function y=rowall(X)
% ROWALL  y = rowall(X)
%         scans each row in X for zeros. If all elements are non-zero a
%         1 is returned in the resulting column vector y, otherwise the
%         corresponding row in y will be zero.
%         X : matrix
%         y : column vector with ones and/or zeros
% example ROWALL([0 1 2;3 4 5])=[0;1]
%         ROWALL([])=1

% file           : rowall.m
% last change    : 13 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[ones(nr,1) X];
y=all(X')';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
