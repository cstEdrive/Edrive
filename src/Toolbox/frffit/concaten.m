function   T=concaten(i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20)
% CONCATEN T = concaten(U,L)
%          concatenates two parts, an upper part U and a lower part L
%          If their row lengthes differ zeros or spaces are added
%          depending on the type of matrices U and L
% < U,L: matrices of free size, either both of number type
%        or both of string type
% > T  : concatenated matrix, either [U O;L] or [U;L O]
%        with O zero or space matrix
% >> concaten(U,L,'right')
%              adds zeros or spaces before U or L such that 
%              U and L will be aligned at the right margin
%
% Similar for more input arguments (to a maximum of 20):
% >> concaten('this','will','be','right','aligned','right')

% file           : concaten.m
% author         : P.Wortelboer
% used functions : concaten
% last change    : 10 november 1997
% version        : 1e

U=[];
T=[];
last=eval(['i' int2str(nargin)]);
if strcmp(last,'right')
  e='right';
  n=nargin-1;
elseif strcmp(last,'left')
  e='left';
  n=nargin-1;
else
  e='left';
  n=nargin;
end
if n==2
  U=i1;
  L=i2;
  [nrU,ncU]=size(U);
  [nrL,ncL]=size(L);
  if isstr(U) & isstr(L)
    if e(1)=='l'
      T=[ [U, setstr(' '*ones(nrU,ncL-ncU))]
        [L , setstr(' '*ones(nrL,ncU-ncL))] ];
    elseif e(1)=='r'
      T=[ [setstr(' '*ones(nrU,ncL-ncU)), U]
        [setstr(' '*ones(nrL,ncU-ncL))], L ];
    end
  else
    if e(1)=='l'
      T=[ [U, zeros(nrU,ncL-ncU)]
        [L , zeros(nrL,ncU-ncL)] ];
    elseif e(1)=='r'
      T=[ [zeros(nrU,ncL-ncU), U]
        [zeros(nrL,ncU-ncL), L] ];
    end
  end
else
  U=i1;
  for i=2:n
    eval(['U=concaten(U,i' int2str(i) ',''' e ''');'])
  end
  T=U;
end

% Copyright Philips Research Labs, 1995,  All Rights Reserved
