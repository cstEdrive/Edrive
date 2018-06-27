function [x,y]=frpinput(n)
% FRPINPUT [x,y]=frpinput(n)
% a robustified version of GINPUT: return key disabled

% file           : frpinput.m
% last change    : 19 augustus 1992
% author         : P.Wortelboer
% used functions :

i=0;
x = [];
y = [];
while i<n
  [xe,ye,button]=ginput(n-i);
  ir=find(button==13);
  if length(ir)>0
    disp(['Hit any other key ' int2str(n) ' times']);
    xe(ir)=[];
    ye(ir)=[];
  end
  x=[x;xe];
  y=[y;ye];
  i=length(x);
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
