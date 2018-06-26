function [Un,hUn]=Unifor(Un0,hUn0)
% UNIFOR [Un,hUn]=Unifor(Un0,hUn0)
%  modifies uniform frequency intervals

% file           : Unifor.m
% author         : P.Wortelboer
% used functions : concaten graphlst
% used in m-file : ffman
% last change    : 30 juli 1998
% version        : 1e+

Un=Un0;
hUn=hUn0;
iplot=get(gca,'UserData');
xlim=get(gca,'XLim');
hF=gcf;
helptxt=concaten(' Pick magnitude      using mouse button 1 (left)',...
                 ' Set  magnitude to 1  with mouse button 2',...
                 ' Quit                   by mouse button 3 or <Esc>');
helptxt=concaten(' ',[' Uniform Weight in axes ' int2str(iplot)],' ',helptxt);
hH=findobj(0,'Type','figure','Name','List');
graphlst(helptxt,hH);
set(findobj(hH,'Type','text'),'FontName','Courier')
set(hH,'Color',1-get(hH,'Color'))

figure(hF)
[w,y,button]=ginput(1);
button=custmkey(button);
if button==1
  Un(iplot)=y;
  set(hUn(iplot),'Xdata',xlim,'Ydata',[y y]);
elseif button==2
  Un(iplot)=1;
  set(hUn(iplot),'Xdata',xlim,'Ydata',[1 1]);
end
set(hH,'Color',1-get(hH,'Color'))
delete(findobj(hH,'Type','text'))

% Copyright Philips Research Labs, 1994,  All Rights Reserved
