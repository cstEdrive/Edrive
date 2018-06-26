function [y,i]=rowmax(X)
% ROWMAX  [y,i] = rowmax(X)
%         finds the maximum in each row of X.
%         X : matrix
%         y : column vector with maxima per row
%         i : column vector with indices
% For row k : y(k)=X(k,i(k))

% file           : rowmax.m
% last change    : 14 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[X -Inf*ones(nr,1)];
[y,i]=max(X');
y=y'; i=i';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
