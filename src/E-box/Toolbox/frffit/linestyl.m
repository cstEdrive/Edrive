function linestyl(h,ls,c)
% LINESTYL  linestyl(h,ls)
% Changes (colored) SOLID lines into lines with styles
% specified in ls
%
% h  : handle vector of lines or axes handle  [] -> gca
% ls : matrix of linestyle rows               [] -> ['- '; '--'; ': '; '-.']
%                                              etc.
% To turn all lines into white: linestyl(h,ls,'w')
% example
% >> h=plot(rand(7));
% >> hl=plotleg(7);
% >> linestyl([h hl],[],'w')   % gives white lines  -  -- :  -. -  -- :
% >> linestyl([h hl],['-';':'] % gives color lines  -  :  -  :  -  :  -
% or
% >> linestyl(findobj(gca,'Type','line','color',[1 0 0]'),':','w')
% >> % to change all solid red lines in current axes to white dotted
% To set all line styles to solid: set(h,'Linestyle','-')

% file           : linestyl.m
% author         : P.Wortelboer
% used functions : 
% last change    : 24 november 1997
% version        : 1e

recolor=1;
if nargin<3
  recolor=0;
  if nargin<2
    ls=[];
    if nargin==0
      h=[];
    end
  end
end
if length(h)==0
  h=gca;
end
if strcmp(get(h(1,1),'Type'),'axes')
  h=sort(findobj(h,'Type','line'));
end
[nr,nc]=size(h);
if length(ls)==0
  ls=['- '; '--'; ': '; '-.'];
end
nl=length(ls(:,1));
for i=1:nr
  k=rem(i,nl);
  if k==0; k=nl; end
  for ii=1:nc
    if strcmp(get(h(i,ii),'LineStyle'),'-')
      if recolor
        set(h(i,ii),'Color',c,'LineStyle',ls(k,:))
      else 
        set(h(i,ii),'LineStyle',ls(k,:))
      end
    end
  end
end

% Copyright Philips Research Labs, 1997,  All Rights Reserved
