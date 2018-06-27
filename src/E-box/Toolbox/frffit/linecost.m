function [co,st]=linecost(cs,to)
% LINECOST    [co,st]=linecost(cs)
% Splits a series of line color/style specifications in
% a column of line color specifications and a column of line
% style specifications.
% < cs: string matrix, each row contains a color spec ('r'
%       or '[1 0 0]' for red for instance), and a linestyle
%       spec ('-' '--' ':' or '-.')
%    ~~~~~Example~~~~~
%    >> cs= ['r- ';'g--']  % cs = strow2cl('r- g--')
%       [co,st]=linecost(cs)
%
% LINECOST can also be used to merge separate color and
% type specifications:
% >> cs=linecost(co,st)

% file           : linecost.m
% author         : P.Wortelboer
% used functions : substrng
% last change    : 23 januari 1998
% version        : 1e


if nargin==1
  [nl,nc]=size(cs);
  co=cs;
  st=cs;
  for i=1:nl
    [c,s,ic]=substrng(cs(i,:),'[]');
    if length(c)>0
      ir=ic(1):ic(2);
    else
      acsi=abs(cs(i,:));
      ir=find(acsi>96 & acsi<123 & acsi~=111 & acsi~=120);
    end
    noir=noni(ir,nc);
    nir=length(ir);
    st(i,:)=[st(i,noir) setstr(32*ones(1,nir))];
    co(i,:)=[co(i,ir)   setstr(32*ones(1,nc-nir))];
  end
  while all(co(:,nc)==' ')
    co(:,nc)=[];
    nc=nc-1;
  end
  i=find(all(st==' '));
  st(:,i)=[];
elseif nargin==2
  co=[cs,to];
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved

