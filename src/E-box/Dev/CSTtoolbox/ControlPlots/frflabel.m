%FRFLABEL Change the labels of frf   
%   
%   Usage: frflabel(handle,{suplabels},{inputlabels},{outputlabels})
%          frflabel(gcf,{'$|G|~\mathrm{in~dB}$','Frequency (Hz)'},{'$u_1$','$u_2$'},{'$y_1$','$y_2$'})
%   
%   Inputs: handle          -   handle of figure
%           suplabels       -   cell of strings with the labels of global axes
%           inputlabels     -   cell of strings with the labels of the inputs             
%           outputlabels    -   cell of strings with the labels of the outputs  
%   
%   Outputs: 
%   
%   See also 

%   References: 
%   
%   Changes: 
%   20110927 - Initial version (M.J.C. Ronde) 
%   
%   
%   M.J.C. Ronde (2011-09-27) 
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.127
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 2798
%   F +31 (0)40 246 1418
%   E m.j.c.ronde@tue.nl

function frflabel(handle,varargin) 
 
% TODO: 
% Make suitable for frf with mag and phase 
 
gtxt = findall(handle,'Type','text'); 
 
% DEBUG 
% for i = 1 : length(gtxt) 
%     get(gtxt(i),'String') 
% end 
 
% Replace superlabels 
if nargin > 1 
    suplabels = varargin{1}; 
     
    if ~isempty(suplabels{1}) 
        magtxt = findall(gtxt,'String','Magnitude [dB]'); 
        set(magtxt,'String',suplabels{1}) 
    end 
    if ~isempty(suplabels{1}) 
        freqtxt = findall(gtxt,'String','Frequency [Hz]'); 
        set(freqtxt,'String',suplabels{2}) 
    end 
end 
 
 
 
 
if nargin > 2 
    inlabels = varargin{2}; 
     
    for i = 1 : length(inlabels) 
        if ~isempty(inlabels{i}) 
            intxt = findall(gtxt,'String',['In:',num2str(i)]); 
            set(intxt,'String',inlabels{i}); 
        end 
    end 
end 
 
 
if nargin > 3 
    outlabels = varargin{3}; 
     
    for i = 1 : length(outlabels) 
        if ~isempty(outlabels{i}) 
            outtxt = findall(gtxt,'String',['Out:',num2str(i)]); 
            set(outtxt,'String',outlabels{i}); 
        end 
    end 
end 
 
 
set(gtxt,'Interpreter','latex') 
set(gtxt,'FontName','Times New Roman') 

