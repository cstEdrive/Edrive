function Y=rowstrin(X,i,r)
% ROWSTRIN  Y = rowstrin(X,i,r)
%           replaces the i'th row of X with r
% X: matrix
% i: row index
% r: row vector of any length
% Y: Y(i,1:length(r))=r
% The columns i+1, etc. of Y are padded with zeros for numerical
% matrices and with ' ' for string matrices
 
% file           : rowstrin.m
% last change    : 6 september 1993
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(X);
l=l(:)';
lr=length(r);
if isstr(X)
  s=' ';
  if lr<nc
    X(i,1:lr)=r;
    X(i,lr+1:nc)=s(1,ones(1,nc-lr));
  else
    X(:,nc+1:lr)=s(ones(nr,1),ones(1,lr-nc));
    X(i,1:lr)=r;
  end
else
  X(i,1:lr)=r;
  if lr<nc
    X(i,lr+1:nc)=zeros(1,nc-lr);
  end
end
Y=X;
 
% Copyright Philips Research Labs, 1994,  All Rights Reserved
