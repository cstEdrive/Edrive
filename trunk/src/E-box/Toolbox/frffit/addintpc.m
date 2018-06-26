function [Qu,hQu]=AddIntpc(Qu0,hQu0)
% ADDINT [Qu,hQu]=AddIntpc(Qu0,hQu0)
%  adds intervals

% file           : AddIntpc.m
% last change    : 4 december 1998
% author         : P.Wortelboer
% modeified      : M.Kruger
% used functions : frgetsf, simlsort
% used in m-file : ffman
% comments       : changed file addint to deal with 2 mouse-buttons instead of three

if nargin==0
  Qu=[];
  hQu=[];
  iplot=1;
elseif nargin==2
  Qu=Qu0;
  hQu=hQu0;
  iplot=find(frgetsf([],'axes',1)==gca);
end
axi=get(gca,'YLim');
xlab=get(get(gca,'xlabel'),'String');
xlabel('mouse buttons: < Pick points | Stop >');
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
  if button~=1
    more=0;
  else

hmarker(1,nint-niq)=line(w,y,'EraseMode','xor','Marker','+');
    [w(2) y(2)]=ginput(1);

hmarker(2,nint-niq)=line(w(2),y(2),'EraseMode','xor','Marker','x');
    [w(3) y(3)]=ginput(1);

hmarker(3,nint-niq)=line(w(3),y(3),'EraseMode','xor','Marker','+');
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
                   'Color','r','LineStyle','--','UserData','psi') iplot];
    nint=nint+1;
  end
end
xlabel(xlab)
nQui=size(Qui);
% intervallen per plot sorteren op eerste frequentie
if nQui(1)>0
  [Qui,dum,i]=simlsort(Qui,Qui(:,4));
  hQui=hQui(i,:);
  Qu=[Qu; Qui];
  hQu=[hQu;hQui];
end
delete(hmarker(:))

% Copyright Philips Research Labs, 1994,  All Rights Reserved

