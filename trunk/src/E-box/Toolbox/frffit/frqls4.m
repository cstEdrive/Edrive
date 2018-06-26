function [G,Zs,Ps,K] = frqls4(Z,struc,WF,option,WFzinit)

% Output-error-LS fit of siso system G to continuous-time and 
% discrete-time frequency response data Z 
% Weighted minimization:
%             criterion = norm([s(jw)-sF]*WF*WFz)    (WFz=zoom-effects)
% < Z     : frequency response data in mu-format
%           (Z=mvfr2vmu(sF,w)=vpck(sF,w); with
%           sF= frequency response data (Np x 1)
%           w = vector of frequencies (Hz!) (if negative --> discrete case)
% < struc : [nden[,nnum[,nint[,nz]]]] structural 'indices' of (s,ns)
%           nden,nnum  = order of denominator,numerator (def: nnum=nden)
%           nint,nz    = number of zeros at jw=0 in den,num
%           ALTERNATIVE: initial estimate (zeros at jw=0 remain fixed)
% < WF    : optional weighting function of length(w) (see criterion above) 
%           ALSO: 0=uniform (default) 1=inv(abs(sF)) 2= as 1 for sF>1
% < option: [iter[,plot[,tol[,nonl]]]] (default: [30,1,1e-7,0];
%           iter = #LS-iteration to approximate OE-estimates 
%           plot = 1/0 = do/don't plot
%           tol  = convergence threshold for 1/den               
%           nonl = 0/1/2 = no/nyq./bode nonl. iterations <-1,-2:only nonl.>
% <WFzinit: initial condition for WFz (optional)              (TUD-WBMR/RS)
% > G     : system in mu-format

% file           : frqls4.M              ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 21 nov 1990
% modified         P. Wortelboer        (c) Philips Research 1994
% modified 	    : M. Kruger, M. Steinbuch
% final update   : 2 april 1991
% used functions : MVFREQN SPLITS BILTRF BODEPLM BODEPLP FRQLSNYQ FRQLSBOD
% last update    : 4 december 1998, april, 21 1999 
% used functions : ope1, unpek, vunpck, zpk2schr
% comments       : adjusted file to deal better with linspace frequency data
%                  this file can deal with 2 mouse-buttons instead of 3
%						 made file compatible with frdiet-tool

global Frfid1Axes Frfid2Axes Frfid3Axes

% check input arguments
if nargin<1,help frqls1,return;end;
if nargin<2,error('not enough input arguments');end;
if nargin>5,error('too many input arguments');end;
% check on optional inputs
if nargin<3,WF=0;end;
if nargin<4,option=[30,1,1e-7,0];end;
if length(option)<2,option=[option,1];end;      % plotshow
if length(option)<3,option=[option,1e-7];end;   % tolden
if length(option)<4,option=[option,0];end;      % nlupdate
iter=option(1);
plotshow=option(2);
tolden=option(3);
nlupdate=option(4);
if length(option)>4,error('check your options (max. 1x4)');end;

% check consistency of input data
[sF,rowpoint,hz]=vunpck(Z);w=hz*2*pi;
sFtot=sF;
sF=mean(sFtot,2);
[np,chk]=size(sF);

%if chk~=1,error('sF should consist of 1 frequency response vector');end
[npw,chk]=size(w);
if npw~=np,error('sF and w should be of equal length');end
if chk~=1,error('specify w as a N x 1 vector of frequencies');end
if (size(WF)~=[np 1])*[1;1]&WF~=0&WF~=1&WF~=2,
  error('weighting function WF must be 0,1 or 2, or match sF');
end;

Psi=[];
if length(WF)==1,
  if WF==0,
    WF=ones(npw,chk);
  elseif WF==1,
    if min(abs(sF))==0,sF=sF+1e-10*max(abs(sF));end;
    WF=abs(sF).^(-1);
  elseif WF==2,
    WF=abs(sF);
    for k=1:np,
      if WF(k)<1,WF(k)=1;end;
    end;
    WF=WF.^(-1);
  end;
else,
  WF=abs(WF);
end;
WF0=WF;
if max(abs(WF-ones(size(WF))))==0,WFuni=1;else,WFuni=0;end;
if nargin==6,
  if size(WFzinit)~=size(w),
    error('Erroneous initial zoom function WFzinit');
  else,
    WFz=WFzinit;
  end;
else,
  WFz=ones(size(WF)); % for zoom
end;

% manipulate w for continuous and discrete time

if max(w)>0, % check for cont. or discrete time
  ccase=1;
  plstyle='loglog';plstylep='semilogx';
  if w(1)<0,
      error('Frequency vector should be <= OR >= 0, not both');
  end;
  wp=w;
else,
  ccase=0; 
  plstyle='plot';plstylep='plot';
  w=-w;
  if w(np)>pi,
    error('You specified a discrete frequency vector beyond pi');
  end;
  while abs(w(np)-pi)<0.01,
    disp('Last frequency point near pi is ignored');
    np=np-1;w=w(1:np);WF=WF(1:np);WFz=WFz(1:np);sF=sF(1:np);
  end;
  wp=w;w=abs((exp(sqrt(-1)*w)-1)./(exp(sqrt(-1)*w)+1));
end;

% delete w = 0
if w(1) == 0
   w(1) = [];
   sF(1) = [];
   WF(1) = [];
	WF0 = WF;
   WFz(1) = [];
   [np,chk]=size(sF);
	[npw,chk]=size(w);
	wp = w;   
end

% in case frequency vector is linear: adjust weighting
if (w(np)-w(np-1))/(w(2)-w(1))<1.02,
   disp('Frequency vector is linear; WF is therefore adjusted.')
   wlog = logspace(log10(w(1)),log10(max(w)),np)';
   Wadj = 1./wlog;
   WF = WF.*Wadj;
end

% normalization of frequency vector
wmax=max(w);
w=w/wmax;

% check struc
nint=0;nz=0; % possibly to be adjusted
[chk,ns]=size(struc);
if chk==1, %struc is structure
  if ns>4,error('check your struc');end;
  mkinit=1;
  nden=struc(1);nnum=nden;
  if ns>1,
    nnum=struc(2);
    if ns>2,
      nint=struc(3);
      if ns>3,
        nz=struc(4);
      end;
    end;
  end;
else,      %struc is initial estimate
  mkinit=0;
  ns=ns-1;
  if ccase==0,
    [struc,ns]=biltrf(struc,ns,'d2c');
  end;
  [a,b,c,d]=splits(struc,ns);
  if size(d)~=[1,1],
    error('Did you specify a siso initial estimate');
  end;
  [num,den]=ss2tf(a,b,c,d,1);
  nnum=length(num)-1;
  nden=length(den)-1;
  % alter initial estimate for normalized frequency vector
  for k=0:nnum,
    num(1+k)=num(1+k)/(wmax^(nden-nnum+k)); 
  end;
  for k=1:nden,
    den(1+k)=den(1+k)/(wmax^(k)); 
  end;
  % check nint
  i=length(den);
  while i>0,
    if den(i)==0,
      nint=nint+1;
    else,
      i=0;
    end;
    i=i-1;
  end
  % check nz
  i=length(num);
  while i>0,
    if num(i)==0,
      nz=nz+1;
    else,
      i=0;
    end;
    i=i-1;
  end
  % check order nnum
  i=1;
  while i<length(num),
    if num(i)==0,
      nnum=nnum-1;
    else,
      i=length(num);
    end;
    i=i+1;
  end
  % check order nden
  i=1;
  while i<nden+1,
    if den(i)==0,
      nden=nden-1;
    else,
      i=nden;
    end;
    i=i+1;
  end;
end;
if nz>nnum,
  error('Structure / initial estimate yields zero-numerator')
end;
if nint>nden,
  error('Structure / initial estimate yields zero-denominator')
end;
if nz>0&nint>0,
  disp('WARNING: numerator and denominator both zero at w=0')
end;
% initial estimate if needed
if mkinit==1,
  disp('initial estimate of FRQLS');
  if nden<10&nnum<10,
    if ccase==1,  
      [num,den]=invfreqs(sF.*WF,w,nnum,nden);
    else,
      num=rand(1,nnum+1);
      den=[1,rand(1,nden)];
    end;
  else,
    num=rand(1,nnum+1);
    den=[1,rand(1,nden)];
  end;
end;
disp('determination of initial errors')
% average relative error: norm(sF-Nest)/norm(sF)
% maximum relative error: max{abs(sF-sest)./abs(sF)}
eF=mvfreqn(num,den,w);
%eF=sF-eF;		% this is the error calculation, here we can input more data!!!!!!!!!!!!!!
eF=sF-eF;
are=norm(eF)/norm(sF);
mre=max(abs(eF)./abs(sF));
itcrit=[are,mre];

 
% build original data matrices PN, RN; PD, RD   'PN=RN * theta'
disp(' ')
disp('Building original data matrix')
disp(' ')
P=zeros(np,1);
R=zeros(np,nden-nint+nnum+1-nz);
iN=nden-nint+1;
[irowR,icolR]=size(R);
PF=[0*P;0*P];
RF=[0*R;0*R];
filF=P;
wi=sqrt(-1)*w;
P=sF.*WF.*(wi.^nden);
for i=nden-1:-1:0+nint,
  R(1:np,nden-i)=-sF.*WF.*(wi.^i);
end;
for i=nnum:-1:0+nz,
  R(1:np,iN+nnum-i)=WF.*wi.^i;
end;
% iterative 'output-error' approximation
istop=0;  % variable istop can report cause of termination
disp('Start output error iterations')
denF=freqs(1,den,w);
format short e
if nlupdate<0,
  istop=5;nlupdate=-nlupdate;
end;
while istop==0,
  denprev=den;
  idx=find(WFz);
  mreden=1;i=0;
  while mreden>tolden&i<iter,
    i=i+1;
    %filter the data at WFz=1 
    filF(1:np,1)=denF;
    PF(idx,1)=real(P(idx,1).*filF(idx,1));
    PF(idx+irowR,1)=imag(P(idx,1).*filF(idx,1));
    for k=1:icolR,
      RF(idx,k)=real(R(idx,k).*filF(idx,1));
      RF(idx+irowR,k)=imag(R(idx,k).*filF(idx,1));
    end;
    % build the least squares problem (incorporate zeros/poles at jw=0):
    theta=RF([idx;idx+irowR],:)\PF([idx;idx+irowR],1);
    % determine new filter_frequency (including zoom)
    % save PREVious DENominator for 1st iter.
    den=[1,theta(1:nden-nint)' zeros(1,nint)];
    denF=freqs(1,den,w);denF=denF.*WFz;
    eF=filF(1:np)-denF;wi=denF;
    areden=norm(eF(idx))/norm(wi(idx));
    mreden=max(abs(eF(idx))./abs(wi(idx)));
    mess=['OE iteration ',int2str(i),'; a/m-re of 1/den: '];
    mess=[mess,num2str(areden),' ',num2str(mreden)];
    disp(mess);
  end;
  if mreden<tolden,istop=1;else,istop=0;end;
  disp('Linear step on numerator coefficients');
  filF(1:np,1)=denF;
  PF(idx,1)=real(sF(idx).*WF(idx));
  PF(idx+irowR,1)=imag(sF(idx).*WF(idx));
  for k=iN:icolR,
    RF(idx,k)=real(R(idx,k).*filF(idx));
    RF(idx+irowR,k)=imag(R(idx,k).*filF(idx));
  end;
  theta=RF([idx;idx+irowR],iN:icolR)\PF([idx;idx+irowR],1);
  % save previous polynomials;
  numprev=num;num=[theta(1:nnum+1-nz)',zeros(1,nz)];
  % calculate criterion 
  sest=mvfreqn(num,den,w);eF=sest-sF;
  are=norm(eF(idx))/norm(sF(idx));
  mre=max(abs(eF(idx))./abs(sF(idx)));
  itcrit=[itcrit;are,mre];
  disp('norm = euclidian norm; error e(w):=sF(w)-s(w);')
  disp('average relative error of s (are) := norm(e)/norm(sF)')
  disp('maximum relative error of s (mre) := max(abs(e)./abs(sF)')
  % mre is an upperbound for the visual difference in bodeplots
  % of sF and s
  disp('are mre for the consecutive iterations:')
  disp(itcrit)
  disp(' ')
  % show results
  if plotshow==1,
    plop=1;
    eF=mvfreqn(numprev,denprev,w);
    axes(Frfid3Axes), cla, set(Frfid3Axes,'Visible','off')
    set(Frfid3Axes,'Visible','off')
    axes(Frfid2Axes)
    bodeplp2(hz,[sFtot sF sest eF],[0 0 2 0],plstylep);
    axes(Frfid1Axes)
    bodeplm2(hz,[sFtot sF sest eF],[0 0],plstyle);
    title('data(solid), cur.est.(dash), prev.est.(dot)')
    if min(WFz)==0,
      hold on;
      plstyleW=[plstylep,'(wp,0.01*ones(size(w))+0.98*WFz,'':'');'];
      eval(plstyleW);
      axis;hold off;
    end;
    pcpause;
%    fig=menux(['another ',int2str(iter),' iterations?'],...
%         'stop',...
%         'repeat estimate loop',...
%         'zoom',...
%         'add weighting function');
%    iplop = 0;
%	 while iplop<1
%  	 	iplop=menux(fig);
%    end
    iplop=menu(['another ',int2str(iter),' iterations?'],...
         'stop',...
         'repeat estimate loop',...
         'zoom',...
         'add weighting function');
	 iplop = iplop-1;
%    close(fig)
%    drawnow
    if iplop==0,nlupdate=0;istop=2;end;
    if iplop==1,istop=0;end;
    if iplop==2&WFuni==0,
%      fig=menux(['Select zoom-window'],...
%         'without weight',...
%         'with weight');
%   		selz=menux(fig)-1;
%      close(fig)
%      drawnow
%      selz=menu(['Select zoom-window'],...
%         'without weight',...
%         'with weight');
%		selz = selz-1;
%	 else,
%      selz=0;
    end;
    while iplop==2, %zoom
	    axes(Frfid1Axes), cla, set(Frfid1Axes,'Visible','off')
	    axes(Frfid2Axes), cla, set(Frfid2Axes,'Visible','off')
	    axes(Frfid3Axes)
%      if selz==0,
        bodeplm2(hz,[sFtot sF sest],[0 0],plstyle);
        title('data(solid) vs. estimate(dash)')
%      else,
%        bodeplm(wp,[sF sest].*(abs(WF)*[1 1]),[0 0],plstyle);
%        title('WEIGHTED data(solid) vs. estimate(dash)')
%      end;  
      if min(WFz)==0,
        hold on;
        plstyleW=[plstylep,'(wp,0.01*ones(size(w))+0.98*WFz,'':'');'];
        eval(plstyleW);
        axis;hold off;
      end;
      ax=axis;
      hold on
      [x,y]=frpinput(2);
      if x(1)>x(2),x=[x(2) x(1)]';end;
      [y,idx1]=min(abs(hz-x(1)));
      [y,idx2]=min(abs(hz-x(2)));
      plstyleW=[plstylep,'([hz(idx1) hz(idx1)],ax(3:4),''-.'')'];
      eval(plstyleW);
      plstyleW=[plstylep,'([hz(idx2) hz(idx2)],ax(3:4),''-.'')'];
      eval(plstyleW);
      pcpause
%          fig=menux(['What to do with this interval ?'],...
%         'Exclude interval',...
%         'Include interval',...
%         'Cancel this interval');
%      x = 0;
%   	while x<1
%  	   	x=menux(fig);
%      end
%      close(fig)
%      drawnow
      x=menu(['What to do with this interval ?'],...
         'Exclude interval',...
         'Include interval',...
         'Cancel this interval');
		x = x-1;
%      fig=menux(['What next ? '],...
%         'Start iterations',...
%         'Another zoom interval',...
%         'Stop iterations');
%      y = 0;
%   	while y<1
%  		   y=menux(fig);
%      end
%      close(fig)
%      drawnow
      y=menu(['What next ? '],...
         'Start iterations',...
         'Another zoom interval',...
         'Stop iterations');
      y = y-1;
      selw = str2num([num2str(y) num2str(x)]);
      if selw>19,iplop=-1;istop=2;else,istop=0;end;
      if selw<10,iplop=-1;end;
      selw=selw-10*floor(selw/10);
      if selw==0,
        WFz(idx1:idx2)=zeros(idx2-idx1+1,1);
      end;
      if selw==1,
        if min(WFz)==1,WFz=zeros(size(WFz));end;
        WFz(idx1:idx2)=ones(idx2-idx1+1,1);
      end;
      hold off
      denF=freqs(1,den,w);denF=denF.*WFz;
    end;
    if iplop==3, %adding weightings
        [WF,Psi]=frqlswpc(sF(:),w(:),Psi,WF0); 
		P=zeros(np,1);
	 	R=zeros(np,nden-nint+nnum+1-nz);
   	iN=nden-nint+1;
	  	[irowR,icolR]=size(R);
 		PF=[0*P;0*P];
   	RF=[0*R;0*R];
   	filF=P;
		wi=sqrt(-1)*w;
		P=sF.*WF.*(wi.^nden);
		for i=nden-1:-1:0+nint,
		  R(1:np,nden-i)=-sF.*WF.*(wi.^i);
		end;
		for i=nnum:-1:0+nz,
		  R(1:np,iN+nnum-i)=WF.*wi.^i;
		end;
		% iterative 'output-error' approximation	 
		denF=freqs(1,den,w);
		format short e
	   istop=0; 
	 end
  else,
    istop=3;
  end;
end; % of the while-loop.
if istop==1,
  disp('Tolerance on maximum relative error of denominator has been met')
  disp(['i.e. max(abs(denF-denprevF)./abs(denF)) < ' num2str(tolden)])
end
% istop==2 : chosen to stop
if istop==3, disp(['One time ',int2str(iter),' iterations']),end
% repair for normalized frequency vector
nnum=length(num)-1;
nden=length(den)-1;
for k=0:nnum,
  num(1+k)=num(1+k)*(wmax^(nden-nnum+k)); 
end;
for k=1:nden,
  den(1+k)=den(1+k)*(wmax^(k)); 
end;
% make sN,sD if proper! (+warning)
if nnum>nden,
  disp('G is non proper --> G:=[num;den]')
%  close(hF)
  s=num;ns=den;
  if ccase==0,
     [Zs,Ps,K]=tf2zp(num,den);
     G=zpk2schr(Zs,Ps,K);
     [s,ns]=unpek(G);	
     [s,ns]=biltrf(s,ns,'c2d');
     [a,b,c,d]=splits(s,ns);
     [ns,s]=ss2tf(a,b,c,d,1);
  end;
  G(1,1:nnum+1)=s;
  G(2,nnum-nden+1:nnum+1)=ns;
  return;
end;
[aa,bb,cc,dd]=tf2ss(num,den);
G = snsys(aa,bb,cc,dd);
%[Zs,Ps,K]=tf2zp(num,den);
%G=zpk2schr(Zs,Ps,K);
if ccase==0,
  [s,ns]=unpek(G);
  [s,ns]=biltrf(s,ns,'c2d');
  G=snsys(s,ns);
  if nint>0
    [s,ns]=unpek(Gi);
    [s,ns]=biltrf(s,ns,'c2d');
    Gi=snsys(s,ns);
    [s,ns]=unpek(Gr);
    [s,ns]=biltrf(s,ns,'c2d');
    Gr=snsys(s,ns);
  end
end;
