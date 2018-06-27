function hgetsf=frgetsf(h,type,s)
% frgetsf  hgetsf=frgetsf(h,'Type',s)
% finds the handles of objects of type 'Type' and optionally
% sorts according to 'Userdata' in the figure with handle h
% h         : figure handle ([] -> current figure)
% type      : 'line', 'text' etc
% s         : 1) sort according to 'Userdata'
 
% file           : frgetsf.m
% last change    : 9 september 1993
% author         : P.Wortelboer
% used functions : SIMLSORT

if length(h)==0
  h=gcf;
end
if nargin<3
  s=0;
end
hgetsf = [];
U = [];
hh=get(h,'Children');
for i=1:length(hh)
  if strcmp(get(hh(i),'Type'),type)
    hgetsf=[hgetsf;hh(i)];
    u=get(hh(i),'UserData');
    if length(u)==0
      s=0;
    else
      if isstr(u)
        u=str2num(u);
      end
      U=[U;u];
    end
  end
end
if s
  hgetsf=simlsort(hgetsf,U);
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
