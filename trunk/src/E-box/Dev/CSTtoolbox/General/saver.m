%SAVER   save a figure 'wysiwyg' in pdf (and optionally in eps)   
%   
%   Usage:   saver(name,fignumber,path,save_eps)
%
%   Inputs:  name:      name of the file (without extension)
%            fignumber: figure number (default: current figure)
%            path:      where to save the file (default: current dir)
%            save_eps:  [ 1 | 0 (default) ] also save as .eps file 
%   
%   Outputs: name.pdf (optionally also name.eps)
%   
%   See also SETPLOT.
   
%   Changes: 
%   20110803 - Initial version (Rob Hoogendijk)
%   20110805 - Changed check for windows/unix computer to work on linux x64 
%              as well, added default value for empty handle  (Michael
%              Ronde)
%   20110817 - Changed error to warning if chosen not to overwrite (Michael
%              Ronde)
%   
%   Rob Hoogendijk (2011-08-3)%   
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.126
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 4227
%   F +31 (0)40 246 1418
%   E r.hoogendijk@tue.nl

function saver(name,handle,path,eps) 
%use current figure if no handle is supplied 
if (nargin<2) || isempty(handle)
    handle=gcf; 
end 
%default path current dir 
if (nargin<3) 
    if(isunix)         
        path=[cd '/']; %for linux users 
    else 
        path=[cd '\']; %for windows users 
    end
% else
%     if path==0
%     path='D:\Thesis\Organization\Presentaties\20111124_Precisiebeurs\'    
%     end
end

%store eps as well 
if(nargin<4) 
    eps=0; 
end 
 
%wysiwyg part... 
%store old units 
oldpaperunits=get(handle,'PaperUnits'); 
oldunits=get(handle,'Units'); 
%set papersize 
set(handle,'PaperUnits','centimeters'); 
set(handle,'Units','centimeters'); 
currentpos = get(handle,'Position'); 
set(handle,'Paperposition',[0 0 currentpos(3) currentpos(4)]); 
set(handle,'PaperSize',[currentpos(3) currentpos(4)]); 
% %restore old units 
set(handle,'PaperUnits',oldpaperunits); 
set(handle,'Units',oldunits); 
 
% save eps 
if(eps) 
    print(handle,'-depsc2',[path,name,'.eps']); 
    disp(['Figure ' num2str(handle) ' stored in ' path,name '.eps']); 
end 
% save pdf 
if exist([path,name,'.pdf'])
   % File does exist, ask what to do
   disp(['Overwrite file ' path, name '.pdf? ']);
   R=input('[y/n]:','s');
    if(R=='y' || isempty(R)) 
        try 
            chan=ddeinit('acroview','control'); 
            if chan 
                ddeexec(chan,[ '[DocOpen("' path , name '.pdf")]' ]); 
                ddeexec(chan,[ '[DocClose("' path , name '.pdf")]' ]); 
            end; 
            delete([path, name '.pdf']) 
        catch 
            delete([path, name '.pdf']); 
        end 
        %warning off; 
        print(handle,'-dpdf',[path,name,'.pdf']);
        %warning on;
        disp(['Figure ' num2str(handle) ' stored in ' path,name '.pdf']);
    else 
        warning('Figure not saved.'); 
    end 
else
    % File does not exist, save
    %warning off;
    print(handle,'-dpdf',[path,name,'.pdf']);
    %warning on;
    disp(['Figure ' num2str(handle) ' stored in ' path,name '.pdf']);
end 

