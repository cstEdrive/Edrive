function [s_out,ns_out]=biltrf(s,ns,txt)

%[s_out,ns_out]=BILTRF(s,ns,txt)
%
%	subroutine to calculate the continuous time state-space matrix from
%   	the discrete time matrix by a bilinear transform or vise versa.
%	inputs:		s, ns	  input time system, number of states
%                      txt       text string
%                                'c2d'   input system continuous
%                                'd2c'   input system discrete
%	outputs:	s_out, ns_out   output time system, number of states
%                                if txt='c2d' discrete
%                                if txt='d2c' continuous
%      Note:           This function combines the functions
%                      BILCD and BILDC.

%	version 1.00        21 juli 1988  P.Heuberger/P.Bongers
%      lab. of Measurement & Control     Delft University of Technology
%     version 2.00        08 mai 1989   P.Bongers
%     converted to PC-MATLAB

if (nargin<2),
  help biltrf
  return
end
if (nargin~=3),
  yesno=0;
  while (yesno==0),
    itxt=input('cont -> discr [0]  or discr -> cont [1]    (0/1): ');
    if (itxt==0|itxt==1),yesno=1;end;
  end;
  if (itxt==0), txt='c2d';else,txt='d2c';end;
end

if (max(size(txt))~=3),
  help biltrf
  return
end

if ns(1,1) == 0,
  s_out=s;
  ns_out=ns;
  return
end

if (txt=='d2c'),                   
  [ad,bd,cd,dd]=splits(s,ns);
  I=eye(ns);
  ac=inv(I+ad)*(ad-I);
  bc=sqrt(2)*inv(I+ad)*bd;
  cc=sqrt(2)*cd*inv(I+ad);
  dc=dd-cd*inv(I+ad)*bd;
  s_out=[ac bc;cc dc];
  ns_out=ns;
elseif (txt=='c2d'),
  [ac,bc,cc,dc]=splits(s,ns);
  I=eye(ns);
  ad=inv(I-ac)*(ac+I);
  bd=sqrt(2)*inv(I-ac)*bc;
  cd=sqrt(2)*cc*inv(I-ac);
  dd=dc+cc*inv(I-ac)*bc;
  s_out=[ad bd;cd dd];
  ns_out=ns;
else,
  help biltrf
  return
end

return

