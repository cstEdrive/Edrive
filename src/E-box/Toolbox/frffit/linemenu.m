function hlinemenu=linemenu(c)
% LINEMENU     hlinemenu=linemenu(c)
% provides a bar of color buttons on top of a picture to change
% line styles of the lines in axes c:
%       by clicking on a specific color button the line of this
%       color changes cyclic 
%
%   ==> ------- ==> - - - - ==> ....... ==> -.-.-.-. ==> 
%        solid      dashed      dotted      dash-dot    invisible
% or
%   ==> +  + ==> o   o ==> *   * ==> .   . ==> x   x ==> 
%       plus     round    asterix    point     cross    invisible
%       
% < c: handle of axes (default: current axes)
%      or vector of line handles
% To get rid of the colored bar:   >> delete(hlinemenu)
% or >> linemenu off
% To change styles in two subplots subsequently
% >> ha1=subplot(2,1,1); plot(....
% >> ha2=subplot(2,1,2); plot(....
% >> h1=linemenu(ha1);
% >> delete(h1)
% >> h2=linemenu(ha2);
%
% Use CLIST to change line styles in all axes of a figure

% file           : ext5/linemenu.m
% author         : P.Wortelboer
% used functions : 
% last change    : 5 mei 1998
% version        : 1e+

hght=0.012;
if nargin==0
  c=gca;
  tp='axes';
elseif strcmp(c,'off')
  % delete previous uicontrols
  h=findobj(gcf,'Type','uicontrol','Tag','linemenu');
  delete(h);
  return
elseif length(c)==1
  if strcmp(get(c,'Type'),'axes')
    tp='axes';
  else
    tp='line';
  end
else
  tp='line';
end
if tp=='axes'
  hl=sort(findobj(c,'Type','line'));
else
  hl=c;
end
n=length(hl);
wdth=1/n;
for k=1:n
  lst=get(hl(k),'LineStyle');
  xs=(k-1)*wdth;
  col=get(hl(k),'Color');
  if any('+o*.x'==lst(1,[1 1 1 1 1]))
    Pos(k,1:4)=[xs 1-hght wdth hght];
    hlinemenu(k)=uicontrol(...
    'Tag','linemenu',...
    'Style','pushbutton','Units','normalized',...
    'String','oooooooooooooooooooooooooooooooooooooooooooooooooooooooooo',...
    'Position',Pos(k,:),...
    'CallBack',...
    ['i=' int2str(k) ';hl=[' sprintf('%.16g ',hl) '];' ...
     'q=get(hl(i),''LineStyle'');v=get(hl(i),''Visible'');' ...
     'if strcmp(q,''+''), if strcmp(v,''on''),set(hl(i),''LineStyle'',''o'');'...
                     'else set(hl(i),''Visible'',''on'');end;' ...
     'elseif strcmp(q,''o''), set(hl(i),''LineStyle'',''*'');' ...
     'elseif strcmp(q,''*''), set(hl(i),''LineStyle'',''.'');' ...
     'elseif strcmp(q,''.''), set(hl(i),''LineStyle'',''x'');' ...
     'elseif strcmp(q,''x''), set(hl(i),''LineStyle'',''+'',''Visible'',''off'');' ...
     'end;'], ...
    'BackGroundColor',col,'Userdata','+');
  else
    Pos(k,1:4)=[xs 1-hght wdth hght];
    hlinemenu(k)=uicontrol(...
    'Tag','linemenu',...
    'Style','pushbutton','Units','normalized',...
    'Position',Pos(k,:),...
    'CallBack',...
    ['i=' int2str(k) ';hl=[' sprintf('%.16g ',hl) '];' ...
     'q=get(hl(i),''LineStyle'');v=get(hl(i),''Visible'');' ...
     'if strcmp(q,''-''), if strcmp(v,''on''),set(hl(i),''LineStyle'',''--'');'...
                     'else set(hl(i),''Visible'',''on'');end;' ...
     'elseif strcmp(q,''--''), set(hl(i),''LineStyle'','':'');' ...
     'elseif strcmp(q,'':''), set(hl(i),''LineStyle'',''-.'');' ...
     'elseif strcmp(q,''-.''), set(hl(i),''LineStyle'',''-'',''Visible'',''off'');' ...
     'end;'], ...
    'BackGroundColor',col,'Userdata','-');
  end
  if strcmp(computer,'PCWIN')
    set(hlinemenu(k),'Style','checkbox')
  end
end

% Copyright Philips CFT, 1998,  All Rights Reserved

