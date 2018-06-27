function  lineeval(oper,hL)
% LINEEVAL   lineeval(oper,hL)
%
% Operates on the X-,Y- or Z-data of a line.
% < oper: matrix string with 'X', 'Y', and/or 'Z' defining the 
%         operation wanted on the X-data, Y-data and/or Z-data.
% < hL  : vector of line handles, or axes or figure handle in
%         which case the operation(s) are applied to all lines 
%         in the axes or figure
%
% To plot a MIMO frequency response in Hz versus dB:
% >> oper=['X/2/pi          ';'20*log10(abs(Y))'];
% >> lineeval(oper,hL);  

% file           : lineeval.m
% last change    : 16 februari 1996
% author         : P.Wortelboer
% used functions : 

[nr,nc]=size(oper);
nl=length(hL);
if nl==1
  if strcmp(get(hL,'Type'),'figure')
    hL=findobj('Type','line');
  elseif strcmp(get(hL,'Type'),'axes')
    hL=findobj(hL,'Type','line');
  end
  nl=length(hL);
end
XYZ=['X';'Y';'Z'];
% to avoid misinterpretations with oper([k;k;k],:):
oper=[setstr(32*ones(1,nc));oper];
for k=2:nr+1
  [ir,ic]=find(oper([k;k;k],:)==XYZ(:,ones(1,nc)));
  for i=1:length(ir)
    eval(['V=''' XYZ(ir(i),1) ''';'])
    eval(['data=''' V 'data'';'])
    for l=1:nl
      eval([V '=get(hL(l),data);'])
      eval(['Dn=' oper(k,:) ';'])
      set(hL(l),data,Dn) 
    end
  end
end

% Copyright Philips Research Labs, 1996,  All Rights Reserved
