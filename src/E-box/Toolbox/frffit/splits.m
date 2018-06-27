function [a,b1,c1,d11,b2,d12,c2,d21,d22] = splits(s,ns)
% SPLITS  [a,b1,c1,d11,b2,d12,c2,d21,d22] = splits(s,ns)
%
% purpose      : split system description into separate matrices
%                dx = Ax  + B1u + B2v
%                 y = C1x + D11u + D12v
%                 z = C2x + D21u + D22v
% method       : -
%
% variables    : [s,ns]  system description ns=[  dim(x)  [dim(v)]
%                                                [dim(z)     -     ]
%                
% (c) DUT-WBMR may 1989                                                (PB)

% file         : SPLITS.M             ver 0.01
% author       : P.Bongers            (c) TUD-WBMR
% last change  : 24 may 1989

[m,n] = size(s);
[r,c]=size(ns);
if ((r==1)&(c==1)),
  dimv=0;dimz=0;ns1=ns;  
elseif ((r==1)&(c==2)),
  dimv=ns(1,2);dimz=0;ns1=ns(1,1);
  b2  = s(1:ns1,n-dimv+1:n);
  d12 = s(ns1+1:m-dimz,n-dimv+1:n);
elseif ((r==2)&(c==2)),
  dimv=ns(1,2);dimz=ns(2,1);ns1=ns(1,1);
  b2  = s(1:ns1,n-dimv+1:n);
  d12 = s(ns1+1:m-dimz,n-dimv+1:n);
  c2  = s(m-dimz+1:m,1:ns1);
  d21 = s(m-dimz+1:m,ns1+1:n-dimv);
  d22 = s(m-dimz+1:m,n-dimv+1:n);
else,
  return;
end
a   = s(1:ns1,1:ns1);
b1  = s(1:ns1,ns1+1:n-dimv);
c1  = s(ns1+1:m-dimz,1:ns1);
d11 = s(ns1+1:m-dimz,ns1+1:n-dimv);

return;
