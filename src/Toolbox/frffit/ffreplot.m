function Qu=ffreplot(hploti,Qu,hQu)
% FFPLOT Qu=ffreplot(hploti,Qu,hQu)  
% replots the quadratic frequency interval functions
% hploti : choice of axes handles
% Qu     : full interval definition
% hQu    : handles for Qu
% If negative function values occur, the interval is reduced
% so to support only positive values. Therefore Qu should be
% returned.

% file           : ffreplot.m
% last change    : 8 september 1993
% author         : P.Wortelboer
% used functions : findx

nplots=length(hploti);
for i=1:nplots
  axes(hploti(i));
  iplot=get(gca,'Userdata');
  xlim=get(gca,'XLim');
  ylim=get(gca,'YLim');
  if length(Qu)>0
    iq=find(Qu(:,6)==iplot);
    niq=length(iq);
    if niq>0
      Qui=Qu(iq,1:6);
      for i=1:niq
        nhe=25;
        whe=logspace(log10(Qui(i,4)),log10(Qui(i,5)),nhe)';
        yhe=Qui(i,1)*ones(nhe,1)+Qui(i,2)*whe+Qui(i,3)*whe.^2;
        gr0=findx(yhe<=0);  % greater than 0
        if length(gr0)>0
          whe=whe(gr0(1,1):gr0(1,2)); 
          yhe=yhe(gr0(1,1):gr0(1,2)); 
          Qui(i,4:5)=whe(gr0(1,:))';
          Qu(iq(i),4:5)=whe(gr0(1,:))';
        end
        whe=[min(whe);whe;max(whe)]; yhe=[ylim(1);yhe;ylim(1)];
        set(hQu(iq(i)),'XData',whe,'YData',yhe,'EraseMode','normal');
        set(hQu(iq(i)),'EraseMode','xor');
      end
    end
  end
end

% Copyright Philips Research Labs, 1993,  All Rights Reserved

