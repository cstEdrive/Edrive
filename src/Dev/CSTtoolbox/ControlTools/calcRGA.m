%CALCRGA Computes the Relative Gain Array (RGA) and RGAnumber  
% interaction measure for a system     
%   
%   Usage: [RGA,RGAno] = calcRGA(G,omega) or RGA = calcRGA(G,omega)    
%   
%   Inputs:  
%   G       -   System or FRD object 
%   omega   -   Frequency vector (rad/s) 
% 
%   Outputs: 
%   RGA     -   FRD object containing RGA(G) 
%   RGAno   -   Vector containing the RGA number 
%   
%   See also 

%   References: 
%   [1] Skogestad, S. and Postlewhaite, I., Multivariable feedback control, 
%   analysis and design, second edition, Wiley, 2005 
%  
%   Changes:  
%   20110722 - Added generalization for non-square systems 
%   20100526 - Number of outputs variable  
%   
%   M.J.C. Ronde (2011-07-22) 
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.127
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 2798
%   F +31 (0)40 246 1418
%   E m.j.c.ronde@tue.nl

function varargout = calcRGA(G,varargin)  
 
if nargin == 1 
    if isa(G,'frd') || isa(G,'idfrd') 
        G       =   chgunits(G,'rad/s'); 
        omega   =   G.Frequency; 
    else 
        omega   =   2*pi*logspace(0,3,1000); 
    end 
     
elseif nargin == 2 
    omega = varargin{1}; 
end 
 
 
for i = 1 : length(omega)  
    [n,m]       =   size(G);  
    Gf          =   freqresp(G,omega(i));  
    if  (n == m) 
        RGAw(:,:,i) =   Gf .* inv(Gf).'; 
        RGAno(i)    =   sum(sum(abs(RGAw(:,:,i)-eye(n))));  
    elseif (n ~= m)  
        RGAw(:,:,i) =   Gf .* pinv(Gf).'; 
    end          
end  
% Convert to FRD object
RGA = frd(RGAw,omega);  
 
% Variable output 
if nargout == 1  
    varargout{1} = RGA;  
elseif nargout == 2  
    varargout{1} = RGA;  
    if (n~=m) 
        % No generalization for RGAno for non-square systems
        RGAno = []; 
    end 
    varargout{2} = RGAno;  
else  
    varargout{1} = RGA; 
end  
  
