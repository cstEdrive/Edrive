%FIGPOS   Gets the size and position of figure(fig) and store on clipboard. 
%   
%   Usage:   figpos(figure)
%
%            example: "figpos(3)" returns the message 
%            Clipboard contains: set(3,'position',[1717 532 560 420]); 
%            Paste this (CTRL-V) after your figure(3); command in your m-file. 
%            The next time you run the m-file, the figure will return to this position/size 
%   
%   Inputs:  figure number (takes current figure if called without argument)
%   
%   Outputs: stores a command on the clipboard
%   
%   See also SET

%   Changes: 
%   20110803 - Initial version (Rob Hoogendijk)
%   20110822 - Renamed pos to figpos (Michael Ronde)	
%   
%   Rob Hoogendijk (2011-08-03)   
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.126
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 4227
%   F +31 (0)40 246 1418
%   E r.hoogendijk@tue.nl

function figpos(fig) 
   if(nargin==0),fig=gcf;end;
    posvec=get(fig,'position');
    clipboard('copy', ['set(',num2str(fig),',''position'',[',num2str(posvec(1,1)),' ',num2str(posvec(1,2)),' ',num2str(posvec(1,3)),' ',num2str(posvec(1,4)),']);'  ])
    disp(['Clipboard contains: ' clipboard('paste')]);
    disp('Paste (CTRL-V) this after your figure(fig); command in your m-file.');
end%function    
