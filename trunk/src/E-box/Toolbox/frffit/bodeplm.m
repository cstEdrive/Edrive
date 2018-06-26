function bodeplm(w1,sF1,w2,sF2,option,style)
% BODEPLM(w1,sF1,w2,sF2,option,style)  generates Magnitude bode plot of 
% complex frequency response sF1 at frequencies w1 (and sF2 resp. w2) 
%
%    - Adds labels 'Magnitude' and 'Frequency [rad/s]'
%    - Enables creation of a legend (optionally)
%    - Inquires for a title (optionally)
%
% INPUTS
% w1,sF1 : vector of frequencies w corresponding to frequency response sF
% w2,sF2 : as w1,sF1  (optional)
% option : [titl [,leg [,label]]]
%          titl  = 1/0 plot title yes/no (default = 0)  
%          leg   = 1/0 plot legend yes/no (default = 0)
%          label = 1/0 other/no labels (default labels above)
% style  : 'loglog' (=default)  'plot' 'semilogx' 'semilogy'
%
% see also BODEPLP

% file           : BODEPLM.M           ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 10 nov 1990
% final update   : 10 nov 1990
% used functions : PLEGEND 

if nargin==0,help bodeplm,return;end
if nargin>6,error('not more than 6 input arguments'),return;end
if nargin<2,error('not enough input arguments'),return;end
[ns1,ni1]=size(sF1);
[nw1,chk]=size(w1);
if chk~=1,error('specify frequency vector w1 as N x 1');end;
if ns1~=nw1,error('w1 and sF1 have unequal number of rows');end;

% check: arg. 3,4 plotdata yes/no
ni2=0;
if nargin==2,
  option=[0 0];
  style='loglog';
end;
if nargin==3,
  option=w2;
  style='loglog';
end;
if nargin==4,
  if isstr(sF2)==1,
    option=w2;
    style=sF2;
  else,
    option=[0 0];
    style='loglog';
    ni2=1;
  end;
end;
if nargin>4,
  ni2=1;
  if nargin==5,
    style='loglog';
  end;
end;
if ni2==1,
  [ns2,ni2]=size(sF2);
  [nw2,chk]=size(w2);
  if chk~=1,error('specify frequency vector w2 as N x 1');end;
  if ns2~=nw2,error('w2 and sF2 have unequal number of rows');end;
end;
[no,nox]=size(option);
if no>1|nox>3,error('specify option as 1 x 1/2/3');end;
if option(1)==1,
  titls=input('Titlestring  ');
  if max(size(titls))==0,
    disp('no title')
    titlx='';
  else,
    titlx='title(titls)';
  end;
else,
  titlx='';
end;
labelx='xlabel(labx);ylabel(laby);';
labx='Frequency [rad/s]';laby='Magnitude';
if nox>1,
  if option(2)==1,
    for i=1:ni1+ni2,
      legt=input(['  explain curve ',int2str(i),': '],'s');
      legtex(i,1:length(legt))=legt;
    end; 
  else,
    legtex='';
  end;
  if nox==3,
    if option(3)==1,
      labx=input('label x-axis: ','s');
      laby=input('label y-axis: ','s');
    else,
      labelx='';
    end;
  end;
end;
plotx=[style,'(w1,abs(sF1)'];
if ni2~=0,
  plotx=[plotx,',w2,abs(sF2))'];
else,
  plotx=[plotx,')'];
end;
% the actual plotting
eval(['hfit=' plotx ';']);eval(labelx);eval(titlx);
linestyl(hfit,['- ';'--';': ';'- ';'--';': ']);
if length(legtex)>0
  [hcolleg,hlines]=colleg(legtex);
end
return;

