function y=rownorm(X,p)
% ROWNORM   y = rownorm(X,p) 
%           calculates NORM for each row in X separately.
%           X : matrix
%           p : 1, 2, inf (default: p=2)
%           y : column vector with norms per row
%           Use ROWNORM(X.',p) to obtain column vector with norms per column

% file           : rownorm.m
% last change    : 10 juli 1992
% author         : P.Wortelboer
% used functions : 

% Frobenius norm equals two norm for vectors
if nargin==1,
  p=2;
end;
[nr,nc]=size(X);
y=zeros(nr,1);
for i=1:nr,
  y(i)=norm(X(i,:),p);
end;
return
% Copyright Philips Research Labs, 1994,  All Rights Reserved
