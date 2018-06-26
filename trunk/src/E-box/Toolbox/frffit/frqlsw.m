function frqlsw(i1,i2,i3,i4,i5)
% FRQLSW  frqlsw(sF,w,frequnit,Psi,wFi)
% Construction of a frequency weighting function on screen
% with help of frequency response plot.
% The current definition (Psi) is adjusted and stored in frqlsw.mat
% after manipulation.
% < sF       : frequency response data             (n x 1)
% < w        : vector of frequencies (rad/s)       (n x 1)
% < frequnit : Units on x-axis of plots, either 'rad/s' or 'Hz'
% < Psi      : user defined frequency weighting data in packed form
%              (see psipek, psuiunpek) containing frequency intervals
%              with quadratic interpolation and a uniform level 
%              [default:  uniform 0.001]
% < wFi      : scalar frequency weighting function (n x 1)
%
% Also allowed
%   >> frqlsw(Z,frequnit,Psi,wFi)
% with Z the mu-format frequency response [Z=frsp(G,hz*2*pi)]

% file           : frqlsw.m
% author         : P.Wortelboer/M.Steinbuch
% used functions : Wor/...
%                  AddInt Delpuin iscalbac psipek psiplot psiunpek
%                  Unifor vmu2mvfr wmshift
% last change    : 31 juli 1998
% version        : 1e+

if iscalbac(i1)==0
  sF=i1;
  if nargin<5
    i5=[];
    if nargin<4
      i4=[];
      if nargin<3
        i3=[];
        if nargin<2
          i2=[];
        end
      end
    end
  end
  if sF(prod(size(sF)))==Inf   % VARYING matrix
    [sF,w]=vmu2mvfr(sF);
    frequnit=i2;
    Psi=i3;
    wFi=i4;
  else
    w=i2;
    frequnit=i3;
    Psi=i4;
    wFi=i5;
  end
  w=w(:);
  npw=length(w);
  if isempty(wFi)
    wFi=ones(size(npw,1));
  end
  if isempty(Psi)
    Un=1;
    Psi=psipek([],[],'1',Un);
    Qu=[];
  else
    [Pu,Qu,Yu,Un]=psiunpek(Psi);
    if strcmp(frequnit,'Hz')
      %%      convert Qu from rad/s data to Hz data      %%
      Qu(:,2)=Qu(:,2)*2*pi;
      Qu(:,3)=Qu(:,3)*4*pi^2;
      Qu(:,[4 5])=Qu(:,[4 5])/2/pi;
    end
  end
  xlbl=get(get(gca,'XLabel'),'String');
  if strcmp(frequnit,'Hz') & ~isempty(findstr(xlbl,'rad/s'))
    oper='X/2/pi';
    lineeval(oper,findobj(gca,'Type','line'));  
    xlabel('frequency [Hz]')
  elseif strcmp(frequnit,'rad/s') & ~isempty(findstr(xlbl,'Hz'))
    oper='X*2*pi';
    lineeval(oper,findobj(gca,'Type','line'));  
    xlabel('frequency [rad/s]')
  end
  set(gca,'UserData',1)
  Psi=psipek([],Qu,'1',Un);
  [hPu,hQu,hUn,Qu]=psiplot(gca,Psi);
  
  savevar4('frqlsw','Qu,hQu,Un,hUn,sF,w,frequnit,Psi,wFi',...
                     Qu,hQu,Un,hUn,sF,w,frequnit,Psi,wFi);

  hui=uimenu(gcf,'Label','Weights...');
  uimenu(hui,'Label','New Intervals',...
         'CallBack','frqlsw(''CallBack1'')');
  uimenu(hui,'Label','Change Uniform level',...
         'CallBack','frqlsw(''CallBack2'')');
  uimenu(hui,'Label','Delete (Int,Uni)',...
         'CallBack','frqlsw(''CallBack3'')');
  uimenu(hui,'Label','Shift  Interval',...
         'CallBack','frqlsw(''CallBack4'')');
  uimenu(hui,'Label','Ready',...
         'CallBack','frqlsw(''CallBack5'')');
elseif iscalbac(i1)==1
  [Qu,hQu]=loadvar4('frqlsw','Qu,hQu');
  [Qu,hQu]=AddInt(Qu,hQu);
  savevar4('frqlsw','Qu,hQu',Qu,hQu);
elseif iscalbac(i1)==2
  [Un,hUn]=loadvar4('frqlsw','Un,hUn');
  [Un,hUn]=Unifor(Un,hUn);
  savevar4('frqlsw','Un,hUn',Un,hUn);
elseif iscalbac(i1)==3
  [Qu,hQu,Un,hUn]=loadvar4('frqlsw','Qu,hQu,Un,hUn');
  [Pu,hPu,Qu,hQu,Un,hUn]=Delpuin([],[],Qu,hQu,Un,hUn);
  savevar4('frqlsw','Qu,hQu,Un,hUn',Qu,hQu,Un,hUn);
elseif iscalbac(i1)==4
  [Qu,hQu]=loadvar4('frqlsw','Qu,hQu');
  [Pu,hPu,Qu,hQu]=wmshift([],[],Qu,hQu);
  savevar4('frqlsw','Qu,hQu',Qu,hQu)
elseif iscalbac(i1)==5
  load frqlsw
  npw=length(w);
  Psi=psipek([],Qu,'1',Un);
  [nqq,nc]=size(Qu);
  if nqq>0 & strcmp(frequnit,'Hz')
    %%     convert Qu back from Hz data to rad/s data     %%
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

  wF=wF.*wFi;

  save frqlsw wF Psi
end

% Copyright Philips CFT, 1998,  All Rights Reserved
