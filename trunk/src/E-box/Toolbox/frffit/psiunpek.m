function   [pu,qu,yu,un]=psiunpek(psi)
% FFUNPEK    [pu,qu,yu,un]=psiunpek(psi)
%
% Unpacks a Frequency Function matrix into its four components:
% pu: PUlses              for each row : [enclosed-area w0 i_psi]
% qu: QUadratic intervals for each row : [m0   m1   m2  w0 w1 i_psi]
% yu: input-output channels of i_psi:    ['u(1:2),y([2 4]),u(4)']
% un: uniform balancing level of i_psi
%
% >> [pu,qu,yu,un]=psiunpek(psi)
% >> psi=psipek(pu,qu,yu,un)


% file           : psiunpek.m
% last change    : 21 februari 1994
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(psi);
if nr==0
  return
end
i=find(psi(:,1)==Inf);
np=psi(i(1),2);
nq=psi(i(1),3);
ny=psi(i(2),2);
nu=psi(i(2),3);
pu=psi(1:np,1:3);
qu=psi([1:nq]+np+1,1:6);
yu=setstr(psi([1:ny]+np+nq+2,2:nc));
un=psi([1:ny]+np+nq+2,1);

% Copyright Philips Research Labs, 1994,  All Rights Reserved
