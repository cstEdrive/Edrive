function [Pu,hPu,Qu,hQu]=veshifpc(Pu,hPu,Qu,hQu)
% WMSHIFT [Pu,hPu,Qu,hQu]=veshifpc(Pu,hPu,Qu,hQu)
%  shift frequency pulses and intervals vertically

% file           : wmshifpc.m
% last change    : 16 november 1993
% author         : P.Wortelboer
% modified       : M.Kruger
% used functions : frgetsf, rownorm, run2nun
% used in m-file : ffman
% comments       : changed file wmshift to deal with 2 mouse-buttons instead of three


iplot=find(frgetsf([],'axes',1)==gca);
if length(Qu)>0
  iq=find(Qu(:,6)==iplot);
  niq=length(iq);
  if niq>0
    Qui=Qu(iq,1:6);
    whq=Qui(:,4);
    yhq=Qui(:,1)+Qui(:,2).*whq+Qui(:,3).*whq.^2;
    for i=1:niq
      hoq(i)=line(whq(i),yhq(i),'EraseMode','xor',...
            'Color','y','LineStyle','o','UserData',i);
    end
  end
else
  niq=0;
end
if length(Pu)>0
  ip=find(Pu(:,3)==iplot);
  nip=length(ip);
  if nip>0
    Pui=Pu(ip,1:3);
    whp=Pui(:,2);
    yhp=Pui(:,1);
    for i=1:nip
      hop(i)=line(whp(i),yhp(i),'EraseMode','xor',...
            'Color','y','LineStyle','x','UserData',i);
    end
  elseif niq==0
    disp(['no frequency function in plot ' int2str(iplot)])
    return
  end
else
  nip=0;
  if niq==0
    disp('no frequency function at all')
    return
  end
end
xlab=get(get(gca,'xlabel'),'String');
button=3;
if nip>0 & niq>0
  xlabel('mouse buttons: < Pick o or x | Stop >');
elseif nip==0
  if niq==1
    button=-1;
  else
    xlabel('mouse buttons: < Pick o | Stop >');
  end
elseif niq==0
  if nip==1
    button=-1;
  else
    xlabel('mouse buttons: < Pick x | Stop >');
  end
end
while button==3
  [xpick,ypick,button]=ginput(1);
end
if button==2
  more=0;
  ivp=[];
  ivq=[];
else
  if button==1
    Xlim=get(gca,'XLim');
    Ylim=get(gca,'YLim');
    xxlog=log10([whq;whp;xpick]);
    yylog=log10([yhq;yhp;ypick]);
    [xxlog,yylog]=run2nun([Xlim Ylim],xxlog,yylog);
    xx=ones(niq+nip,1)*[xxlog(nip+niq+1) yylog(nip+niq+1)];
    X=[xxlog(1:niq+nip) yylog(1:niq+nip)];
    [dum,iv]=min(rownorm(X-xx));
  else 
    iv=1;
  end
  if iv<=niq
    type='o';
    ivq=iv;
    xpick=whq(ivq);
    ypick=yhq(ivq);
    xright=Qu(iq(ivq),5);
    yright=Qu(iq(ivq),1)+Qu(iq(ivq),2).*xright+Qu(iq(ivq),3).*xright.^2;
    xmid=10^((log10(xpick)+log10(xright))/2);
    ymid=Qu(iq(ivq),1)+Qu(iq(ivq),2).*xmid+Qu(iq(ivq),3).*xmid.^2;
    x3pick=[xpick xmid xright];
    y3pick=[ypick ymid yright];
    x3=x3pick;
    y3=y3pick;
    Xpick=get(hQu(iq(ivq),1),'XData');
    Ypick=get(hQu(iq(ivq),1),'YData');
    Qukpick=Qu(iq(ivq),:);
    Quk=Qukpick;
    ivp=[];
  elseif iv>niq & iv<=niq+nip
    type='x';
    ivp=iv-niq;
    xpick=whp(ivp);
    ypick=yhp(ivp);
    Xpick=get(hPu(ip(ivp),1),'XData');
    Ypick=get(hPu(ip(ivp),1),'YData');
    Puk=Pu(ip(ivp),:);
    ivq=[];
  end
  npnts=length(Ypick);
  more=1;
  xlabel(['mouse buttons: ',...
          '< vertical shift | Accept & Stop >']);
  m_shift=0;
  w_shift=0;
end
if nip>0
% if length(ivp)>0, set(hop(ivp),'EraseMode','normal'); end
  for i=noni(ivp,nip)'
    delete(hop(i))
  end
end
if niq>0
% if length(ivq)>0, set(hoq(ivq),'EraseMode','normal'); end
  for i=noni(ivq,niq)'
    delete(hoq(i))
  end
end
while more
  [xsh,ysh,butsh]=ginput(1);
  if type=='o'
    if butsh==1
      m_shift=ysh-ypick;
      y3=y3pick+m_shift;
      Quk(1:3)=y3/[ones(1,3); x3 ; x3.^2];
      Quk=ffreplot(gca,Quk,hQu(iq(ivq)));
      set(hoq(ivq),'YData',ysh);
    else
      more=0;
      Qu(iq(ivq),1)=Quk(1);
      Qu(iq(ivq),4:5)=Quk(4:5);
      delete(hoq(ivq))
    end
  else
    if butsh==1
      m_shift=ysh-ypick;
      set(hPu(ip(ivp),1),'YData',Ypick+m_shift*[1 0]);
      set(hop(ivp),'YData',ysh);
    else
      more=0;
      Pu(ip(ivp),2)=Puk(2)+w_shift;
      Pu(ip(ivp),1)=Puk(1)+m_shift;
      delete(hop(ivp))
    end
  end
end
xlabel(xlab)

% Copyright Philips Research Labs, 1994,  All Rights Reserved
