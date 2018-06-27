function [hPu,hQu,hUn,Qu]=frpsiplo(hplot,Pu,Qu,Un)
% FRPSIPLO [hPu,hQu,hUn,Qu]=frpsiplo(hplot,Pu,Qu,Un)  
% plots the pulses, the quadratic frequency interval functions
% and the uniform weights
% hplot : all axes handles
% Pu    : full pulse definition
% Qu    : full interval definition
% Un    : full uniform weight definition
% hPu, hQu, hUn   : handles for Pu, Qu, Un
% If negative function values occur in an interval, the interval 
% is reduced so to support only positive values. Therefore Qu 
% should be returned.
% >> [hPu,hQu,hUn,Qu]=frpsiplo(hplot,psi)
% is also allowed for psi the packed frequency weight definition
% (see psipek, psiunpek).

% file           : frpsiplo.m
% last change    : 10 november 1994
% author         : P.Wortelboer
% used functions : deltpall, psiunpek

if nargin==2
  [Pu,Qu,Yu,Un]=psiunpek(Pu);
end
hcA=gca;
nplots=length(hplot);
lUn=length(Un);
if lUn==0
  Un=zeros(nplots,1);
end
for iplot=1:nplots
  axes(hplot(iplot));
  deltpall(hplot(iplot),'Type,line,Color,[1 0 0]');
  xlim=get(gca,'XLim');
  ylim=get(gca,'YLim');
  if iplot<=lUn
    hUn(iplot)=line(xlim,Un(iplot,[1 1]),'LineStyle',':','Color','r',...
             'EraseMode','xor','UserData','psi');
  end
  if length(Pu)>0
    id=find(Pu(:,3)==iplot);
    nid=length(id);
    if nid>0
      Pui=Pu(id,1:3);
      hPu(id,1:2)=[line(Pui(:,[2 2])',[Pui(:,1)';ylim(1)*ones(1,nid)],...
      'EraseMode','xor','Color','r','UserData','psi') ones(nid,1)*iplot];
    end
  else
    nid=0;
    hPu=[];
  end
  if length(Qu)>0
    iq=find(Qu(:,6)==iplot);
    niq=length(iq);
    if niq>0
      Qui=Qu(iq,1:6);
      for i=1:niq
        nhe=25;
        whe=logspace(log10(Qui(i,4)),log10(Qui(i,5)),nhe)';
        yhe=Qui(i,1)*ones(nhe,1)+Qui(i,2)*whe+Qui(i,3)*whe.^2;
        i0=min(find(yhe<=0));
        if length(i0)>0
          whe=whe(1:i0-1); yhe=yhe(1:i0-1); 
          Qu(iq(i),5)=whe(i0-1);
        end
        whe=[min(whe);whe;max(whe)]; yhe=[ylim(1);yhe;ylim(1)];
        hQu(iq(i),1:2)=[line(whe,yhe,'EraseMode','xor',...
        'Color','r','LineStyle','--','UserData','psi') iplot];
      end
    end
  else
    niq=0;
    hQu=[];
  end
end
axes(hcA);

% Copyright Philips Research Labs, 1994,  All Rights Reserved

