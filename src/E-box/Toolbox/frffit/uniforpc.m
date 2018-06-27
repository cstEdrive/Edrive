function [Un,hUn]=Uniforpc(Un0,hUn0)
% UNIFOR [Un,hUn]=Uniforpc(Un0,hUn0)
%  modifies uniform frequency intervals

% file           : Uniforpc.m
% last change    : 4 december 1998
% author         : P.Wortelboer
% modified       : M. Kruger
% used functions : frgetsf
% used in m-file : ffman
% comments       : changed file unifor to deal with 2 mouse-buttons instead of three


Un=Un0;
hUn=hUn0;
iplot=find(frgetsf([],'axes',1)==gca);
xlim=get(gca,'XLim');
xlab=get(get(gca,'xlabel'),'String');
xlabel('mouse buttons: < Pick magnitude | Quit >');
[w,y,button]=ginput(1);
if button==1
  Un(iplot)=y;
  set(hUn(iplot),'Xdata',xlim,'Ydata',[y y]);
end
xlabel(xlab)

% Copyright Philips Research Labs, 1994,  All Rights Reserved
