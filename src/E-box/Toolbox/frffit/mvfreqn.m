function [sF]=mvfreqn(num,den,w)
% [sF]=mvfreqn(num,den,w)
%
% determination of complex frequency response sF of the system 
% (num,den) with one input for all frequencies specified in w.
%
% sF is ordered as:
%                 u1         
%             [y1 ... yn ] 
%

% file           : MVFREQN.M           ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 4 oct 1990
% final update   : 12 feb 1990
% used functions : SPLITS

if min(size(w))~=1,disp('specify a vector of frequencies w'),return,end
if size(den)==[1 1]&size(num)==[1 1],
  re=(num/den)*ones(w);
  im=zeros(w);
else,
  while den(1)==0,
    den=den(2:length(den));
    if den==0,
      error('Denominator is zero')
    end;
  end;      
  while num(1)==0,
    num=num(2:length(num));
    if num==0,
      sF=zeros(w);return;
    end;
  end;      
  if length(den)<length(num),
    chk=den;
    den=num;
    num=chk;
    chk=1;
  else,
    chk=0;    
  end;
  [re,im]=nyquist(num,den,w);
end;
if isempty(im)
  sF=re;
else
  sF=re+sqrt(-1)*im;
end
if chk==1,
  sF=sF.^(-1);
end
return;
