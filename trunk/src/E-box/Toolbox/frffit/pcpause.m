% PCPAUSE   generates a pause on PC's only (not under Windows). 
%           This is necessary after plot commands to allow inspection.
%           For computers with graphics windows a pause is
%           confusing and PCPAUSE does not generate a PAUSE.

% file           : pcpause.m
% last change    : 18 mei 1994
% author         : P.Wortelboer
% used functions :

q=computer;
if strcmp(q,'PC') |strcmp(q,'PCAT') | strcmp(q,'PC386') 
  pause
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
