function [num,den]=frqlsnyq(sF,w,wp,num,den,nz,nint,option,WF,plstyle);
% [num,den]=frqlsnyq(sF,w,wp,num,den,nz,nint,option,WF*WFz,plstyle)
% Nonlinear NYQUIST update of estimate in FRQLS
%                                                           (TUD-WBMR/RS)

% file           : FRQLSNYQ.M              ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 15 feb 1991
% final update   : 28 mar 1991
% used functions : MVFREQN, BODEPLM, BODEPLP

disp('Begin NONLINEAR estimation');
maxiter=option(1);
plotshow=option(2);
tolJ=option(3);
rho=0.78;
stepfac=0.002;
np=length(w);
plstyleW=plstyle;
if length(plstyle)==6,
  if plstyle=='loglog',plstylep='semilogx';end;
elseif length(plstyle)==8,
  if plstyle=='semilogx',plstylep='semilogx';end;
else,
  plstylep='plot';
end;
if max(WF-ones(size(WF)))==0,
  WFuni=1;
else,
  WFuni=0;
  if min(abs(WF))==0,
    if length(plstyle)==6,
      if plstyle=='loglog',
        plstyleW='semilogx';
        disp(['plotstyle weighted-error is semilogx']);
      end;
    elseif length(plstyle)==8,
      if plstyle=='semilogy',
        plstyleW='plot';
        disp(['plotstyle weighted error minimization has been changed']);
      end;
    end;    
  end;
end;

if plotshow==1,
  nltimes=pi;
  while floor(nltimes)~=nltimes,
   nltimes=input(['How many times 5 nonlinear and 1 linear update? --> ']); 
  end;
  if nltimes<1,return;end;
else,
  nltimes=1000;
end;
nden=length(den)-1;
nnum=length(num)-1;N=max(nnum,nden);
thetaA=den(2:length(den)-nint)';nA=length(thetaA);
thetaB=num(1:length(num)-nz)';nB=length(thetaB);

% creation of "internal macros"
calP   = 'P=mvfreqn([thetaB'',zeros(1,nz)],[1,thetaA'',zeros(1,nint)],w);';
calJ   =['eval(calP);'...
         'Jfac=(sF-P).*WF;'...
         'Je=sum(abs(Jfac).^2);'];
% build fixed data matrices and establish algorithm constants
wi=sqrt(-1)*w;
Fmtx=zeros(np,nB); 
for k=1:nB,
  Fmtx(:,k)=wi.^(nnum+1-k);
end;
gradB=zeros(thetaB);
gradA=zeros(thetaA);
grad2B=gradB;
grad2A=gradA;
nonliter=5;
% begin estimation loop with linear step
format short e
eval(calJ);
Jold=Je;Joldlin=Je;
eval(calP);
Pprev=P;
% beginistop
istop=0;
while istop==0,
  conv=0;titer=0;
  while conv==0,
    iter=0;inf=0;titer=titer+1;
    while iter<nonliter&inf<2,
      iter=iter+1;
      % calculate gradients
      eval(calP);kB=1;kA=1;
      for k=N:-1:0,
        Zk=mvfreqn([zeros(1,N-k),1,zeros(1,k)],...
                   [zeros(1,N-nden),1,thetaA',zeros(1,nint)],w);
        if k<=nnum,
          if k>=nz,
            gradB(kB)=-2*sum(real(conj(sF-P).*Zk));
            grad2B(kB)=2*sum(abs(Zk).^2);
            kB=kB+1;
          end;
        end;
        if k<nden,
          if k>=nint,
            gradA(kA)=2*sum(real(conj(sF-P).*P.*Zk));
            grad2A(kA)=sum(2*(abs(P).^2).*(abs(Zk).^2)...
                           -4*real(conj(sF-P).*P.*Zk.*Zk));
            kA=kA+1;
          end;
        end;
      end;
      % save previous parameter values
      thetaAold=thetaA;thetaBold=thetaB;
      if iter==1,gradB=0*gradB;end; % no update of B after linstep
      ixgA=find(grad2A>0);ixgB=find(grad2B>0);
      thetaA(ixgA)=thetaA(ixgA)-gradA(ixgA)./grad2A(ixgA);
      thetaB(ixgB)=thetaB(ixgB)-gradB(ixgB)./grad2B(ixgB);
      Jold=Je;eval(calJ);
      niter=0;
      while Je>Jold&niter<maxiter, % NEED of 'backtracking'
        niter=niter+1;
        thetaA=thetaAold+rho*(thetaA-thetaAold);
        thetaB=thetaBold+rho*(thetaB-thetaBold);
        eval(calJ);
      end;
      if Je>Jold,  % no update over grad2i>0
        Je=Jold;inf=1;thetaA=thetaAold;thetaB=thetaBold;
      else, % try improvement by extra "backtracking"...?
        niter=0;Jold=Je;
        while Je<=Jold&niter<maxiter, % EXTRA 'backtracking'
          niter=niter+1;
          thetaA=thetaAold+rho*(thetaA-thetaAold);
          thetaB=thetaBold+rho*(thetaB-thetaBold);
          Jold=Je;eval(calJ);
        end;
        if Je>Jold, % set 1 step back
          thetaA=thetaAold-(thetaAold-thetaA)/rho;
          thetaB=thetaBold-(thetaBold-thetaB)/rho;
          Je=Jold;
        end;
        inf=0;
      end;
      ixgxA=find(grad2A<=0);
      if size(ixgxA)~=[0,0], % search over rest A-parameters
        thetaAold=thetaA;   %protect recently updated..
        gradA(ixgA)=0*gradA(ixgA); % A-parameters
        ixgxxA=find(grad2A<0);
        gradA(ixgxxA)=-gradA(ixgxxA)./grad2A(ixgxxA);
        ixgxxA=find(gradA~=0);
        if size(ixgxxA)~=[0,0], % linstep over A may be profitable
          stepvA=thetaAold(ixgxxA)./gradA(ixgxxA);
          stepA=stepfac*min(abs(stepvA(find(stepvA))));
          if size(stepA)==[0,0],stepA=0.01;end;
          niter=-1;Jold=2*Je;
          while Je<Jold&niter<maxiter,
            niter=niter+1;
            thetaA=thetaAold-stepA*gradA*(rho^(-niter));
            Jold=Je;eval(calJ);
          end;
          if Je>Jold,   % A linstep search stopped over Je
            Je=Jold;
            if niter==0,  % no update over A linstep
              inf=inf+1;thetaA=thetaAold;
            else,
              thetaA=thetaAold-stepA*gradA*(rho^(-niter));
            end;
          end;
        else, % no update over A linstep
          inf=inf+1; 
        end;
      else,
        inf=inf+1; % no grad2A<=0;
      end;
      ixgxB=find(grad2B==0);
      if size(ixgxB)~=[0,0], % search over rest B-parameters
        thetaBold=thetaB;  % protect recently updated..
        gradB(ixgB)=0*gradB(ixgB); % B-parameters
        ixgxxB=find(gradB~=0);
        if size(ixgxxB)~=[0,0], %linstep over B may be profitable
          stepvB=thetaBold(ixgxxB)./gradB(ixgxxB);
          stepB=stepfac*min(abs(stepvB(find(stepvB))));
          if size(stepB)==[0,0],stepB=0.01;end;
          niter=-1;Jold=2*Je;
          while Je<Jold&niter<maxiter,
            niter=niter+1;
            thetaB=thetaBold-stepB*gradB*(rho^(-niter));
            Jold=Je;eval(calJ);
          end;
          if Je>Jold, % stop via no_decrease_of_Je
            Je=Jold;
            if niter==0, % no linstep update of B
              thetaB=thetaBold;
            else,
              thetaB=thetaBold-step*gradB*(rho^(-niter));
              inf=inf-1;
            end;
          end;
        end;
      end;
      if inf==2,
        disp(['no improvement possible at nonlinear step ',...
               int2str(titer),'_',int2str(iter)]);
      else,
        disp(['nonlin.iter ',int2str(titer),'_',...
               int2str(iter),'; Je = ',sprintf('%.8g',Je)]);
      end;
    end;
    % linear step.
    PF=[real(sF.*WF);imag(sF.*WF)];
    Zt=mvfreqn(1,[1,thetaA',zeros(1,nint)],w);
    RF=[real(Fmtx.*[(WF.*Zt)*ones(1,nB)]);imag(Fmtx.*[(WF.*Zt)*ones(1,nB)])];
    idx=[find(WF);np+find(WF)];
    thetaB=RF(idx,:)\PF(idx,1);
    eval(calJ);
    Jrel=(Joldlin-Je)/Je;
    disp(['linear update ',int2str(titer),'; Je = ',sprintf('%.8g',Je),...
          '; Jrel = ',sprintf('%.8g',Jrel)]);
    if Jrel<tolJ,
      conv=1;
    else,
      Joldlin=Je;
    end;
    if plotshow==1,
      if titer==nltimes,conv=1;end;
    end;
  end;
  eval(calP);
  showF=[sF,P,Pprev];Pprev=P;
  if plotshow~=1,
    istop=3;
  else, %display results
    plop=1;
    clg;subplot(221);
    bodeplm(wp,showF,0,plstyle);
    title('data - estimate - previous estimate');
    if WFuni==0,
      bodeplm(wp,showF.*(WF*ones(1,3)),0,plstyleW);
      title('weighted data and estimates');
    end;
    subplot(223);
    bodeplp(wp,showF,0,plstylep);
    c_type = computer;
    if c_type(1:3)~='VAX',pause,end; 
    iplop=-1;
    while min(abs(iplop-[0 1 2]))~=0,
      disp(['  choose out of the following options ']);
      disp('     0. stop');
      disp('     1. repeat estimate loop');
      disp('     2. change number of iterations_at_a_stroke');
      iplop=input(['make your choice --> ',7,7]);
    end;
    if iplop==0,istop=2;end;
    if iplop==1,istop=0;end;
    if iplop==2,
      nltimes=pi;
      while floor(nltimes)~=nltimes,
        nltimes=input(['Nr. of iterations_at_a_stroke? --> ']); 
      end;
      if nltimes<1,istop=2;end;
    end;
  end; % end of plotshow
end; % of the while-istop-loop.
den=[1,thetaA',zeros(1,nint)];
num=[thetaB',zeros(1,nz)];
