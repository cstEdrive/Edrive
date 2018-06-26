%FRF: Plot Magnitude Frequency Response Function
%
%   Usage:   frfmag(sys1,'color/linetype/marker',sys2,'color/linetype/marker',...
%            ...{fmin,fmax,magmin,magmax,phasemin,phasemax,wraponoff,type})
%
%   Inputs:  You can use custom colors, e.g. lightgrey, pink, etc. (see line 180)
%            example strings: 'b', 'lightgrey--*', 'orange', '-^'
% 
%            All elements in the options-struct are optional 
%            fmin/fmax in Hz
%            magmin/magmax in dB
%            phasemin/phasemax in degrees
%            wraponoff: [ 'on' | 'off' (default) ]
%            type: [ 'mag' (default) | 'phase' | 'magphase'  ]
%
%   Outputs: Plots the frf in the current figure.
%            Note that the xticklabels might overlap the xlabel in the
%            figure. In an exported pdf file they will be correct.
%   
%   See also FRF, FRFPHASE, SETPLOT.

%   Changes: 
%   2011083 - Initial version (Rob Hoogendijk)
%
%   Rob Hoogendijk (2011-08-3) 
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.126
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 4227
%   F +31 (0)40 246 1418
%   E r.hoogendijk@tue.nl

function frfmag(varargin)

typeisset=0;
for ii=1:length(varargin)
    if(~iscell(varargin{ii}))
        argin{ii}=varargin{ii};
    else
        options=varargin{ii};        
        options{8}='mag';
        argin{ii}=options;
        typeisset=1;
    end
end

if(~typeisset)
    options{8}='mag';
    argin{ii+1}=options;
end

frf(argin{1:end});

end