function bodeplp(w1,sF1,w2,sF2,option,style)
% BODEPLP(w1,sF1,w2,sF2,option,style)  generates Phase bode plot of 
% complex frequency response sF1 at frequencies w1 (and sF2 resp. w2) 
%
%    - Adds labels 'Phase' and 'Frequency [rad/s]'
%    - Enables creation of a legend (optionally)
%    - Inquires for a title (optionally)
%
% INPUTS
% w1,sF1 : vector of frequencies w corresponding to frequency response sF
% w2,sF2 : as w1,sF1  (optional)
% option : [titl [,leg [,label [,unwrap]]]]
%          titl  = 1/0 plot title yes/no (default = 0)  
%          leg   = 1/0 plot legend yes/no (default = 0)
%          label = 1/0 other/no labels (default labels above)
%          unwrap= 1/0 unwrap the phase beyond [-pi,pi] yes/no (default = 0)  
% style  : 'loglog' 'plot' 'semilogx' 'semilogy' (=default)
%
% see also BODEPLM                                     (TUD-WBMR/RS)

% file           : BODEPLP.M           ver 1.01
% author         : R. Schrama           (c) TUD-WBMR, 10 nov 1990
% final update   : 18 dec 1990
% used functions : PLEGEND ANGLEDEG

if nargin==0,help bodeplp,return;end
if nargin>6,error('not more than 6 input arguments'),return;end
if nargin<2,error('not enough input arguments'),return;end
[ns1,ni1]=size(sF1);
[nw1,chk]=size(w1);
if chk~=1,error('specify frequency vector w1 as N x 1');end;
if ns1~=nw1,error('w1 and sF1 have unequal number of rows');end;

% check: arg. 3,4 plotdata yes/no
ni2=0;
if nargin==2,
  option=[0 0 2 0];
  style='semilogx';
end;
if nargin==3,
  option=w2;
  style='semilogx';
end;
if nargin==4,
  if isstr(sF2)==1,
    option=w2;
    style=sF2;
  else,
    option=[0 0 2 0];
    style='semilogx';
    ni2=1;
  end;
end;
if nargin>4,
  ni2=1;
  if nargin==5,
    style='semilogx';
  end;
end;
if ni2==1,
  [ns2,ni2]=size(sF2);
  [nw2,chk]=size(w2);
  if chk~=1,error('specify frequency vector w2 as N x 1');end;
  if ns2~=nw2,error('w2 and sF2 have unequal number of rows');end;
end;
[no,nox]=size(option);
if no>1|nox>4,error('specify option as 1 x 1/2/3/4');end;
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
labx='Frequency [rad/s]';laby='Phase';
if nox>1,
  if option(2)==1,
    for i=1:ni1+ni2,
      legt=input(['  explain curve ',int2str(i),': '],'s');
      legtex(i,1:length(legt))=legt;
    end; 
  else,
    legtex='';
  end;
  if nox>2,
    if option(3)==1,
      labx=input('label x-axis: ','s');
      laby=input('label y-axis: ','s');
    else,
      if option(3)==0,labelx='';end;
    end;
  end;
  if nox==4,
    unwopt=option(4);
  else,
    unwopt=0;
  end;
end;
plotx=[style,'(w1,angledeg(sF1,unwopt)'];
if ni2~=0,
  plotx=[plotx,',w2,angledeg(sF2,unwopt))'];
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

