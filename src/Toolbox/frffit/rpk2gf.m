function gf = rpk2gf(R,P,K)
% RPK2GF  gf = rpk2gf(R,P,K)
% makes a complex modal system realization for a siso system
% in Residue form (see RESIDUE)
% R : residues
% P : poles
% K : direct feedthrough term (D)
% gf: complex modal form (scaled, sorted) 

% file           : rpk2gf.m
% last change    : 18 november 1993
% author         : P.Wortelboer                Philips Research
%                    wortel@prl.philips.nl     The  Netherlands
% used functions : simlsort

i=find(R==0);
if length(i)>0
  disp('Fraction(s) with zero Residue removed')
  R(i)=[];
  P(i)=[];
end
n=length(P);
S=sqrt(abs(R));
F=[P S conj(R./S)];
cr=[100*eps;1e-8];
F=simlsort(F,imag(F(:,1)));
L=F(:,1);
p=atan2(real(L),imag(L));
ireal=sort([find(abs(abs(p)-.5*pi)<cr(1))
            find(real(L)==0&imag(L)==0)]);
nr=length(ireal);
ncp=(n-nr)/2;
if ncp>0
  ij=[1:ncp];
  ji=[n:-1:n-ncp+1];
  F(ji,1:3)=conj(F(ij,1:3));
end
if nr>0
  index=ncp+1:ncp+nr;
  F(index,:)=real(simlsort(F(index,:),real(F(index,1))));
end
if (~length(K))
  gf(n+1,1) = 0; 
else
  gf(n+1,1)=K; 
end
gf(n+1,[2 3])=[nr+j*(ncp+1e-300) -Inf];
gf(1:n,:)=F;

% Philips Research Labs, 1994
