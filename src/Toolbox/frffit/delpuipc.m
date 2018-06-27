function [Pu,hPu,Qu,hQu,Un,hUn]=Delpuipc(Pu,hPu,Qu,hQu,Un,hUn)
% DELPUIPC [Pu,hPu,Qu,hQu,Un,hUn]=Delpuipc(Pu,hPu,Qu,hQu,Un,hUn)
%  delete frequency pulses, intervals and sets uniform level
%  back to zero

% file           : Delpuipc.m
% last change    : 4 december 1998
% author         : P.Wortelboer
% modified       : M.Kruger
% used functions : frgetsf
% used in m-file : FFMAN
% comments       : changed file delpuin to deal with 2 mouse-buttons instead of three

iplot=find(frgetsf([],'axes',1)==gca);
if length(Qu)>0
  iq=find(Qu(:,6)==iplot);
  niq=length(iq);
  if niq>0
    Qui=Qu(iq,1:5);
    whq=Qui(:,4);
    yhq=Qui(:,1)+Qui(:,2).*whq+Qui(:,3).*whq.^2;
    for i=1:niq
      hoq(i)=line(whq(i),yhq(i),'EraseMode','xor',...
            'Color','y','LineStyle','o');
    end
  end
else
  niq=0;
end
if length(Pu)>0
  ip=find(Pu(:,3)==iplot);
  nip=length(ip);
  if nip>0
    Pui=Pu(ip,1:3)
    whp=Pui(:,2);
    yhp=Pui(:,1);
    for i=1:nip
      hop(i)=line(whp(i),yhp(i),'EraseMode','xor',...
            'Color','y','LineStyle','x');
    end
  elseif niq==0
    disp(['no frequency function in plot ' int2str(iplot)])
  end
else
  nip=0;
  if niq==0
    disp('no frequency function at all')
  end
end
xlab=get(get(gca,'xlabel'),'String');
if niq>0 & nip>0
  xlabel('mouse buttons: < Pick o,x''s | Stop >');
elseif niq==0 & nip==0
  xlabel('mouse buttons: <   | Stop >');
elseif niq==0
  xlabel('mouse buttons: < Pick x''s | Stop >');
elseif nip==0
  xlabel('mouse buttons: < Pick o''s | Stop >');
end
more=1;
Xlim=get(gca,'XLim');
Ylim=get(gca,'YLim');
[xxlog,yylog]=run2nun([Xlim Ylim],log10([whq;whp]),log10([yhq;yhp]));
X=[xxlog yylog];
Ivq = [];
Ivp = [];
while more
  [x,y,button]=ginput(1);
  if button~=1
    more=0;
  else
    [xlog,ylog]=run2nun([Xlim Ylim],log10(x),log10(y));
    x=ones(niq+nip,1)*([xlog ylog]);
    [dum,iv]=min(rownorm(X-x));
    X(iv,:)=Inf*[1 1];
    if iv<=niq
      delete(hQu(iq(iv)));
      delete(hoq(iv));
      hoq(iv)=0;
      Ivq=[Ivq;iv];
    else
      iv=iv-niq;
      delete(hPu(ip(iv)));
      delete(hop(iv));
      hop(iv)=0;
      Ivp=[Ivp;iv];
    end
    if length(Ivp)==nip & length(Ivq)==niq
      more=0;
    end
  end
end
xlabel(xlab)
if length(Ivp)>0
  hPu(ip(Ivp),:)=[];
   Pu(ip(Ivp),:)=[];
end
for i=1:length(hop)
  if hop(i)>0
    delete(hop(i))
  end
end
if length(Ivq)>0
  hQu(iq(Ivq),:)=[];
   Qu(iq(Ivq),:)=[];
end
for i=1:length(hoq)
  if hoq(i)>0
    delete(hoq(i))
  end
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
