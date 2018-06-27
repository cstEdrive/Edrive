function sysn = snsys(sys,be,C,D,T,Cv,Ca)
% SNSYS  builds or transforms a SYSTEM. Several types of inputs are
%        allowed. The output is a SYSTEM in Sn-form
%
% Input of state-space matrices:  >> G=snsys(A,B,C,D)
%       of the SN-form:           >> G=snsys(S,n)
%          the DE-form:           >> G=snsys(D,E)
%          the DF-form:           >> G=snsys(D,F)
%       or the DH-form:           >> G=snsys(D,H)
%       of mechanical matrices:   >> G=snsys(M,L,K,F,Cx,Cv,Ca)
%    or of SYSTEM matrices:       >> G=snsys(G)
%                                      snsys(Ge)
%                                      snsys(Gf)
%                                      snsys(Gh)
%                                      snsys(Gm)
% An extra scalar input argument is assumed to be the sampling time
% for discrete-time systems.
%
% Note that only the DE- and DF-form are unique for almost all
% systems.
%
% [S,n]=unpek(sys) returns the (S,n) matrices of the Sn-form
%
% See also: sosys, desys, dfsys, dhsys, unpek

% file           : snsys.m
% author         : P.Wortelboer
% used functions : de2sn, df2sn, dhsys, dtpek, gh2sys, snsys, sort12, 
%                  so2sn, unpek
% last change    : 3 november 1997
% version        : 1e

global SPARSE_SYSTEMS
if exist('SPARSE_SYSTEMS')~=1    % not necessary anymore 
  SPARSE_SYSTEMS=0;              % in 
end                              % MATLAB5
if isempty(SPARSE_SYSTEMS)
    SPARSE_SYSTEMS=0;
end

if nargin>5
  [sys,be]=so2sn(sys,be,C,D,T,Cv,Ca);
elseif nargin==5	% A,B,C,D,T of DISCRETE-TIME SYSTEM	(disc)
  sysn=snsys([sys be;C D],length(sys));
  sysn=dtpek(sysn,T);
  return
elseif nargin==4	% A,B,C,D of CONTINUOUS-TIME SYSTEM     (cont)
  T=0;
  n=length(sys);
  sys=[sys be;C D];
  be=n;
elseif nargin==3        % (S,n), (D,E), (D,F), or (D,H) and T   (disc)
  sysn=snsys(sys,be);
  sysn=dtpek(sysn,C);
  return
elseif nargin==2
  if issys(sys)         % sys and T                             (disc)
    sysn=snsys(sys);
    sysn=dtpek(sysn,be);
    return
  else                  % (S,n), (D,E), (D,F), or (D,H)         (cont)
    T=0;
    if ~isempty(sys)
      [ny,nu]=size(sys);
      [n,nn]=size(be);
      if nn==1+nu+ny			% DF-form  or  DH-form
        if any(imag(be(:,1))<0)		% DF-form
          [bs,i]=sort12(be(:,1)/j);
          if all(i==[1:n]')
            [sys,be]=df2sn(sys,be);
          else
            [sys,be]=df2sn(sys,be(i,:));
          end
        else				% DH-form
          [sys,be]=unpek(gh2sys(dhsys(sys,be)));
        end
      else				% DE-form
        [sys,be]=de2sn(sys,be);
      end
    end
  end
elseif nargin==1
  [sys,T]=dtpek(sys);
  [nr,nc]=size(sys);
  if sys(nr,nc)~=-Inf
    help snsys
    error('sys NOT a SYSTEM matrix in one of forms Sn, SO, DE, DF, DH')
  end
  n=sys(nr,nc-1);
  if n~=0      % DF- or DE-form
    [sys,be]=unpek(sys); 
    [ny,nu]=size(sys);
    [n,nn]=size(be);
    if nn==1+nu+ny   % DF-form  or  DH-form
      if any(imag(be(:,1))<0)      % DF-form
        [bs,i]=sort12(be(:,1)/j);
        if all(i==[1:n]')
          [sys,be]=df2sn(sys,be);
        else
          [sys,be]=df2sn(sys,be(i,:));
        end
      else                        % DH-form
        [sys,be]=unpek(gh2sys(dhsys(sys,be)));
      end
    else             % DE-form
      [sys,be]=de2sn(sys,be);
    end
  else
    if sys(1,nc)<0
      [M,L,K,F,Cx,Cv,Ca]=unpek(sys);
      [sys,be]=so2sn(M,L,K,F,Cx,Cv,Ca);
    else
      sysn=dtpek(sys,T);
      return
    end
  end
end
[nr,nc]=size(sys);
if SPARSE_SYSTEMS==1
  sysn=sparse(nr+1,nc+1);
else
  sysn=zeros(nr+1,nc+1);
end
sysn(nr+1,nc+1)=-Inf;
if nr>0
  sysn(1:nr,1:nc)=sys;
  sysn(1,nc+1)=be;
  sysn=dtpek(sysn,T);
end

% Copyright Philips Research Labs, 1996,  All Rights Reserved
