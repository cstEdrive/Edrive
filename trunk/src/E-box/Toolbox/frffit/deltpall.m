function deltpall(h,str)
% DELTPALL  deltpall(h,str)
% deletes all objects within figure or axes that match the
% specification in str
% Within the Children of h a search is performed
% h    : handle(s) of axes or figure
% str  : string of types, styles etc.
% Example    deltpall(gca,'Type,line,Color,[1 1 1]')
%            deletes all white lines form the current axes

% file           : deltpall.m
% author         : P.Wortelboer
% used functions : deltpall, strow2cl
% last change    : 5 november 1997
% version        : 1e

H=h;
h=h(1);
hh=get(h,'Children');
Str=strow2cl(str,',',31);
[nr,nc]=size(Str);
levels=nr/2;
nhh=length(hh);
bool=ones(nhh,1);
for i=1:nhh
  hi=hh(i);
  for il=1:levels
    if bool(i)
      A=Str(2*il-1,:);
      A(abs(A)==31)=[];
      B=Str(2*il,:);
      B(abs(B)==31)=[];
      C=get(hi,A);
      if ~isstr(C)
        if any(B=='[')
          eval(['B=' B ';'])
          if all(C==B)
            % OK
          else
            bool(i)=0;
          end
        else
          bool(i)=0;
        end
      else
        if strcmp(C,B)
        % OK
        else
          bool(i)=0;
        end
      end
    end
  end
end
delete(hh(find(bool)))

nH=length(H);
for ih=2:nH
  deltpall(H(ih),str);
end

% Copyright Philips Research Labs, 1994,  All Rights Reserved
