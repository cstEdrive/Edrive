%SCHROEDER Calculate multisine with Schroeder phase scaled to max|u|=1    
%   
%   Usage: [t,U,f] = schroeder(f,fs,NFFT,P)
%   
%   Inputs:  
%   f       -   Frequencies, leave empty for full spectrum
%   fs      -   Sample frequency (Hz)
%   NFFT    -   Block length
%   P       -   Number of periods
%   
%   Outputs: 
%   t       -   Time vector
%   U       -   Designed multisine
%   f       -   Frequencies (Hz)
%   
%   See also 

%   References: 
%   
%   Changes: 
%   20110630 - Corrected sin to cos to get lower crest factor 
%   20110624 - Scaled maximum amplitude to 1 
%   2010???? - Initial version 
%    
%   M.J.C. Ronde (2011-06-30) 
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.127
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 2798
%   F +31 (0)40 246 1418
%   E m.j.c.ronde@tue.nl

function [t,U,f] = schroeder(varargin) 

if nargin < 4 
    disp('Number of arguments must be 4') 
else 
    f       =   varargin{1}; 
    fs      =   varargin{2}; 
    NFFT    =   varargin{3}; 
    P       =   varargin{4}; 
end 
ts = 1 / fs; 
 
 
% Frequencies 
if isempty(f) 
    disp('Standard frequency grid') 
    f  =   (fs/2*linspace(0,1,1+NFFT/2))'; 
end 
% Remove zero frequency 
if f(1) == 0 
    f = f(2:end); 
end 
 
% Calculate phases 
phi = zeros(length(f),1); 
for k = 1 : length(f) 
    phi(k) = -((k*(k-1))/length(f))*pi; 
end 
 
% Calculate multisine 
t = (0 : ts : (NFFT-1)*ts).'; 
u = zeros(length(t),1); 
A = ones(length(f),1); 
for i = 1 : length(t) 
    for k = 1 : length(f) 
        u(i) = u(i)+A(k)*cos(2*pi*f(k)*t(i)+phi(k)); 
    end 
end 
 
% Scale by the maximum amplitude 
u = u ./ max(abs(u)); 
 
% Multiple periods 
t = (0 : ts : (P*NFFT-1)*ts).'; 
U = []; 
for j = 1 : P 
    U = [U; u]; 
end 
    
 
     
     
