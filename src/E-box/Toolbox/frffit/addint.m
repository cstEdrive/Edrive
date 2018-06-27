function [Qu,hQu]=AddInt(Qu0,hQu0)
% ADDINT [Qu,hQu]=AddInt(Qu0,hQu0)
%  adds intervals

% file           : AddInt.m
% author         : P.Wortelboer
% used functions : concaten custmkey graphlst simlsort
% used in m-file : ffman
% last change    : 30 juli 1998
% version        : 1e+

if nargin==0
  Qu=[];
  hQu=[];
  iplot=1;
elseif nargin==2
  Qu=Qu0;
  hQu=hQu0;
  iplot=get(gca,'UserData');
end
hF=gcf;
axi=get(gca,'YLim');
helptxt=concaten(' Pick points  using mouse button 1 (left)',...
                 ' Stop         by other mouse button or <Esc>');
helptxt=concaten(' ',[' Add Interval Weights in axes ' int2str(iplot)],...
                 ' ',helptxt);
hH=findobj(0,'Type','figure','Name','List');
graphlst(helptxt,hH);
set(findobj(hH,'Type','text'),'FontName','Courier')
set(hH,'Color',1-get(hH,'Color'))

figure(hF)

if length(hQu)>0
  iq2=find(hQu(:,2)==iplot);
else
  iq2=[];
end
if length(Qu)>0
  iq=find(Qu(:,6)==iplot);
else
  iq=[];
end
niq2=length(iq2);
niq=length(iq);
first=1;
if ~isempty(iq) & ~isempty(iq2)
  if niq~=niq2
    first=0;
  elseif any(iq~=iq2)
    first=0;
  end
end
if first==0, error('Intervals and their Line Handles mixed up');
end
if niq>0
  hQui=hQu(iq,:);
  Qui=Qu(iq,:);
  hQu(iq,:)=[];
  Qu(iq,:)=[];
end
more=1;
nint=1+niq;
while more
  [w,y,button]=ginput(1);
  button=custmkey(button);
  if button>=2
    more=0;
  else

    hmarker(1,nint-niq)=line(w,y,'EraseMode','xor','LineStyle','+');
    [w(2) y(2)]=ginput(1);

    hmarker(2,nint-niq)=line(w(2),y(2),'EraseMode','xor','LineStyle','x');
    [w(3) y(3)]=ginput(1);

    hmarker(3,nint-niq)=line(w(3),y(3),'EraseMode','xor','LineStyle','+');
    [y,w]=simlsort(y,w);
    if any(diff(w)==0)
      w(2)=(w(1)+w(3))/2;
      y(2)=(y(1)+y(3))/2;
    end
    Qui(nint,1:6)=[y/[ones(1,3); w; w.^2] w(1) w(3) iplot];
    nhe=25;
    whe=logspace(log10(Qui(nint,4)),log10(Qui(nint,5)),nhe)';
    yhe=Qui(nint,1)*ones(nhe,1)+Qui(nint,2)*whe+Qui(nint,3)*whe.^2;
    i0=min(find(yhe<=0));
    if length(i0)>0
      whe=whe(1:i0-1); yhe=yhe(1:i0-1);
      Qui(nint,5)=whe(i0-1);
    end
    whe=[min(whe);whe;max(whe)]; yhe=[axi(1);yhe;axi(1)];
    hQui(nint,1:2)=[line(whe,yhe,'EraseMode','xor',...
                   'Color','w','LineStyle','--','UserData','psi') iplot];
    nint=nint+1;
  end
end
nQui=size(Qui);
% intervallen per plot sorteren op eerste frequentie
if nQui(1)>0
  [Qui,dum,i]=simlsort(Qui,Qui(:,4));
  hQui=hQui(i,:);
  Qu=[Qu; Qui];
  hQu=[hQu;hQui];
end
delete(hmarker(:))
set(hH,'Color',1-get(hH,'Color'))
delete(findobj(hH,'Type','text'))

% Copyright Philips Research Labs, 1994,  All Rights Reserved

