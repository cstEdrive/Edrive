function [y,i]=rowmin(X)
% ROWMIN  [y,i] = rowmin(X)
%         finds the minimum in each row of X.
%         X : matrix
%         y : column vector with minima per row
%         i : column vector with indices
% For row k : y(k)=X(k,i(k))

% file           : rowmin.m
% last change    : 14 juli 1992
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
X=[X Inf*ones(nr,1)];
[y,i]=min(X');
y=y'; i=i';
% Copyright Philips Research Labs, 1994,  All Rights Reserved
