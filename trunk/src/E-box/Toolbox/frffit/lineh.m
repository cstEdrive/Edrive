function h=lineh(linestyle)
% LINEH  h=lineh(linestyle)
%        h=lineh
% finds the line handles in the current axes
%        h=lineh('-.')
% finds the -. line handles

% file           : lineh.m
% last change    : 7 september 1993
% author         : P.Wortelboer
% used functions :

hh=get(gca,'Children');
for i=1:length(hh)
  if strcmp(get(hh(i),'Type'),'line')
    if nargin==1
      if strcmp(get(hh(i),'LineStyle'),linestyle)
        h=[h;hh(i)];
      end
    else
      h=[h;hh(i)];
    end
  end
end

% Copyright Philips Research Labs, 1993,  All Rights Reserved
