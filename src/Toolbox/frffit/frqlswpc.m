function [wF,Psi]=frqlswpc(sF,w,Psi,wFi)
%[wF,Psi]=frqlswpc(sF,w,Psi,wFi)
%Construction of a frequency weighting function on screen
%with help of frequency response plot.
%The current definition (Psi) is adjusted
% sF     : frequency response data             (n x 1)
% w      : vector of frequencies (rad/s)       (n x 1)
% Psi    : user defined frequency weighting data in packed form
%          (see psipek, psuiunpek) containing frequency intervals
%          with quadratic interpolation and a uniform level 
%          [default:  uniform 0.001]
%
% wFi    : scalar frequency weighting function (n x 1)
%
% Also allowed
% >> [wF,Psi]=frqlswpc(Z,Psi)
% with Z the mu-format frequency response [Z=frsp(G,hz*2*pi)]

% file           : frqlsw.m
% last change    : 29 november 1994
% author         : P.Wortelboer/M.Steinbuch
% used functions : /home/wortel/matlab/mu_syn
%                  minfo, vunpck
%                : /home/wortel/matlab/Wor
%                  AddInt, Delpuin, psipek, frpsiplo, psiunpek
%                  Unifor, wmshift
%                : /home/wortel/matlab/ext4
%                  menux

[t,ny,nu,npw]=minfo(sF);
if nargin<4, wFi=ones(size(w)); end
if strcmp(t,'vary')
  if nargin==2
    Psi=w;
  else
    Psi=[];
  end
  [sF,rp,w]=vunpck(sF);
  ny=rp(2)-1;
  for i=1:ny
    SF=[SF sF(rp+i-1,:)];
  end
  sF=SF;
else
  npw=length(w);
  if nargin==2
    Psi=[];
  end
  rp=1:npw;
end
if length(Psi)==0
   Un=1;
   Psi=psipek([],[],'1',Un);
   Qu = [];
   Pu = [];
else
  [Pu,Qu,Yu,Un]=psiunpek(Psi);
%%      convert Qu from rad/s data to Hz data      %%
	if ~isempty(Qu)	
		Qu(:,2)=Qu(:,2)*2*pi;
		Qu(:,3)=Qu(:,3)*4*pi^2;
      Qu(:,[4 5])=Qu(:,[4 5])/2/pi;
   end
end
hll=loglog(w/2/pi,abs(sF));
xlabel('frequency [Hz]')
set(gca,'UserData',1)
Psi=psipek([],Qu,'1',Un);
[hPu,hQu,hUn,Qu]=frpsiplo(gca,Psi);

index=1;
cf=gcf;
%fig=menux('Choose option',...
%         'New Intervals',...
%         'Change Uniform level',...
%         'Delete (Int,Uni)',...
%         'Shift  Interval horizontally',...
%         'Shift  Interval vertically',...
%         'Ready');
while index>0
   i = menu('Choose option',...
         'New Intervals',...
         'Change Uniform level',...
         'Delete (Int,Uni)',...
         'Shift  Interval horizontally',...
         'Shift  Interval vertically',...
         'Ready');
      while i == 0
      end
%  i=menux(fig);
%  figure(cf);
  if i==1
    [Qu,hQu]=AddIntpc(Qu,hQu);
  elseif i==2
    [Un,hUn]=Uniforpc(Un,hUn);
  elseif i==3
    [Pu,hPu,Qu,hQu,Un,hUn]=...
               Delpuipc(Pu,hPu,Qu,hQu,Un,hUn);
  elseif i==4
    [Pu,hPu,Qu,hQu]=hzshifpc(Pu,hPu,Qu,hQu);
  elseif i==5
    [Pu,hPu,Qu,hQu]=veshifpc(Pu,hPu,Qu,hQu);
  elseif i==6
    index=-1;
  end
  index=index+1;
end
Psi=psipek([],Qu,'1',Un);
[nqq,nc]=size(Qu);
%%     convert Qu from Hz data to rad/s data     %%
if nqq>0
  Qu(:,2)=Qu(:,2)/2/pi;
  Qu(:,3)=Qu(:,3)/4/pi^2;
  Qu(:,[4 5])=Qu(:,[4 5])*2*pi;
end

wF=ones(npw,1)*Un;
for i=1:nqq
  k=find(w>=Qu(i,4)&w<=Qu(i,5));
  l=length(k);
  if l>0 
    whe=w(k);
    yhe=Qu(i,1)*ones(l,1)+Qu(i,2)*whe+Qu(i,3)*whe.^2;
    i0=find(yhe<0);
    yhe(i0)=zeros(length(i0),1);
    wF(k)=wF(k)+yhe;
  end
end

Psi=psipek([],Qu,'1',Un);

%delete(fig);
figure(cf);

wF=wF.*wFi;

% Copyright Philips Research Labs, 1994,  All Rights Reserved
