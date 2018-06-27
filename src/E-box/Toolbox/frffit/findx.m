function ii=findx(v,x)
% FINDX  ii=findx(v,x)
% 
% finds blocks of x within a vector v. Matrix ii has two
% columns of which the first points to the start and the
% second to the end of a block.
% v: vector
% x: scalar [default: 0]
% ii: Two-column matrix with start-end pointers
%
% >> ii=findx([1 2 0 0 5 0 7 0 0 0])
% ii =
%      3     4
%      6     6
%      8    10
% >> r=rand(1,5),ii=findx(r>.5,1)
% gives the location of the elements of r that are larger than .5
% >> ii=findx('double  spacing',' ')
% ii =
%      7     8


% file           : findx.m
% last change    : 22 november 1994
% author         : P.Wortelboer
% used functions : rowall

if nargin<2
  x=0;
end
n=length(v);
e=1:n;
noi=find(v~=x);
e(noi)=zeros(1,length(noi));
E=[0 e(1:n-1);zeros(n+1,n)];
for i=1:n+1
  E(i+1,1:n+1-i)=e(i:n);
end
for i=1:n
  K=find(E(:,i)==0);
  if K(1)==1
    ii(i,1:2)=[E(K(1)+1,i) E(K(2)-1,i)];
  end
end
ii(rowall(ii==0),:)=[];

% Copyright Philips Research Labs, 1994,  All Rights Reserved
