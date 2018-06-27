function y=rowany(X)
% ROWANY  y = rowany(X)
%         scans each row in X for any non-zero element. The resulting
%         column vector contains a 1 for each row with a non-zero element
%         and a 0 for rows with only zeros.
%         X : matrix
%         y : column vector with ones and/or zeros
% example ROWANY([0 1;0 0])=[1;0]
%         ROWANY([])=0

% file           : rowany.m
% last change    : 13 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[zeros(nr,1) X];
y=any(X')';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
