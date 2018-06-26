function   [S,n,C,D,Cx,Cv,Ca,T]=unpek(sys,c)
% UNPEK    [S,n]=unpek(G)
%
%   Unpacks a SYSTEM matrix in Sn-, DE-, DF-, DH-, or So-form
%   into its describing matrices (S,n), (D,E), (D,F), (D,H),
%   (M,L,K,F,Cx,..) 
% >>  [S,n]=unpek(G);            % - packed by SNSYS
% >>  [A,B,C,D]=unpek(G);        % /
% >>  [D,E,nrcp]=unpek(Ge);      %   packed by DESYS
% >>  [D,F,nrcp]=unpek(Gf);      %   packed by DFSYS  
% >>  [D,H,nrcp]=unpek(Gh);      %   packed by DHSYS 
%        with nrcp [#real-poles, #complex-conjugate-pole-pairs]
%
% >>  [M,L,K,F,Cx,..]=unpek(Gso); % for mechanical systems
%                                 % packed by SOSYS
% For DISCRETE-TIME SYSTEMS an extra output argument is returned:
% the sampling time T.
%
% It is also possible to extract a single matrix:
% >> C=unpek(G,'C');
% This option is available for 'A','B','C','D','E','F','G','H','S'.
%
% See also: unpck, snsys, sosys, desys, dfsys, dhsys

% file           : Wor/unpek.m
% author         : P.Wortelboer
% used functions : Wor/...
%                  diax e2xbc f2lbc issys unpek
% last change    : 16 juli 1998
% version        : 1e+

matlabversion=version;
matlabversion=str2num(matlabversion(1));

if matlabversion>4
  eval(['ok=isstruct(sys);'])
  if ok
    eval(['sys=getfield(sys,''SYSTEM'');'])
  end
end


if nargin == 0 | nargin > 2
  help unpek
  return
elseif nargin==1
  c=' ';
end
i012=issys(sys);
if isstr(i012)
  if strcmp(i012,'s')
    sys=destruc(sys);
    i012=issys(sys);
  end
end
[nr,nc]=size(sys);
  
if real(i012)~=1
  help unpek
  if i012==0
    error('sys NOT a SYSTEM matrix in Sn-, DE-, DF-, DH-, or So-form')
  else
    error('sys contain MULTIPLE SYSTEMs: use selsys')
  end
end
[sys,T]=dtpek(sys);
if nr==1 & nc==1
  S=[];
  if c==' '
    if T==0 
      if nargout==2
        n=0;
      elseif nargout==3
        C=[0 0];
      end
    else
      if nargout<=3
        n=0;
        C=T;
      elseif nargout==4
        C=[0 0];
        D=T;
      else
        Cx=T;
      end
    end
  end
  return
end
N=sys(nr,nc-1);
if N==0  % So- or Sn-form
  n=sys(1,nc);
  if n<0   % So-form: mechanical system
    n=-n;
    nu=nc-3*n-1;
    ny=nr-n-1;
    i=1:n;
    M=sys(i,i);
    L=sys(i,i+n);
    K=sys(i,i+2*n);
    F=sys(i,(3*n+1):(nc-1));
    Ca=sys(n+1:nr-1,i);
    Cv=sys(n+1:nr-1,i+n);
    Cx=sys(n+1:nr-1,i+2*n);
    S=M;
    n=L;
    C=K;
    D=F;
  else      % Sn form
    S=sys(1:nr-1,1:nc-1);
    if c==' '
      if nargout==3
        C=T;
      elseif nargout==4 | nargout==5
        D=S(n+1:nr-1,n+1:nc-1); C=S(n+1:nr-1,1:n);
        B=S(1:n,n+1:nc-1); S=S(1:n,1:n); n=B;
        if nargout==5
          Cx=T;
        end
      end
    else
      if c=='A'
        S=S(1:n,1:n);
      elseif c=='B'
        S=S(1:n,n+1:nc-1);
      elseif c=='C'
        S=S(n+1:nr-1,1:n);
      elseif c=='D'
        S=S(n+1:nr-1,n+1:nc-1);
      elseif c~='S'
        error('Specified output matrix not available in input SYSTEM')
      end
    end
  end
else        % DE, DF or DH form
  if imag(N)>0  % DF or DH form
    nrcp=round([real(N) imag(N)]);
    nr_cp=nrcp(1)+nrcp(2);
    n=nrcp(1)+2*nrcp(2);
    C=nrcp;
    if any(imag(sys(1:nr_cp,1))<0)  % DF form
      systype='DF';
      nu=nc-1-nr+n;
      S=sys(n+1:nr,1:nu);
      n=sys(1:n,1:nc);
      if c=='F'
        S=n;
      elseif c~='D' & c~=' '
        error('Specified output matrix not available in input SYSTEM')
      end
    else                            % DH form
      systype='DH';
      nu=nc-1-nr+nr_cp;
      S=sys(nr_cp+1:nr,1:nu);
      n=sys(1:nr_cp,1:nc);
      if c=='H'
        S=n;
      elseif c~='D' & c~=' '
        error('Specified output matrix not available in input SYSTEM')
      end
    end
    if nargout==4
      D=T;
    end
  else                % DE form
    n=round(N);
    nrcp=sys(nr,nc-2);
    C=[nrcp (n-nrcp)/2];
    nu=nc-2-nr+n;
    S=sys(n+1:nr,1:nu);
    n=sys(1:n,1:nc);
    if c=='E'
      S=n;
    elseif c~='D' & c~=' '
      error('Specified output matrix not available in input SYSTEM')
    end
    if nargout==4
      D=T;
    end
  end
end

% Copyright Philips CFT, 1998,  All Rights Reserved
