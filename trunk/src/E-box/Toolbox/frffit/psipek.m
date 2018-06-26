function   [psi]=psipek(pu,qu,yu,un)
% PSIPEK    [psi]=psipek(pu,qu,yu,un)
%
%   Packs a Frequency Function matrix with three components:
% pu: PUlses              for each row : [enclosed-area w0 i_psi]
% qu: QUadratic intervals for each row : [m0   m1   m2  w0 w1 i_psi]
% yu: input-output channels of i_psi:    ['u(1:2),y([2 4]),u(4)']
% un: uniform balancing level of i_psi
%
% >> [pu,qu,yu,un]=psiunpek(psi)
% >> psi=psipek(pu,qu,yu,un)

% file           : psipek.m
% last change    : 21 februari 1994
% author         : P.Wortelboer
% used functions : concaten

[np,nc]=size(pu);
[nq,nc]=size(qu);
yu=abs(yu);
[ny,nu]=size(yu);
psi=concaten([[pu;Inf np nq] zeros(np+1,3);qu;Inf ny nu 0 0 0],...
            [un yu]);

% Copyright Philips Research Labs, 1994,  All Rights Reserved
