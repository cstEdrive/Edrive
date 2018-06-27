function  Z=mvfr2vmu(mvfr,w)
% MFR2VMU  Z = mvfr2vmu(mvfr,w)
%
% Packs the multivariable frequency response matrix
% mvfr and its associated frequency vector w in the
% 'varying matrix' Z
% < mvfr: a column of component matrices. The i'th component
%         corresponds to the i'th element of w
%         ('multivariable frequency response' matrix)
% <    w: vector ('frequency vector') 
% >    Z: VARYING matrix
%
% See also  vmu2mvfr, vmuget, vmueval, vmuput

% file           : mvfr2vmu.m
% last change    : 1 december 1994
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(mvfr);
nw=length(w);
Z=zeros(nr+1,nc+1);
Z(1:nr,1:nc)=mvfr;
Z(1:nw,nc+1)=w(:);
Z(nr+1,nc+1)=Inf;
Z(nr+1,nc)=nw;

% Copyright Philips Research Labs, 1994,  All Rights Reserved
