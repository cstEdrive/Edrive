%FRF: Plot Frequency Response Function
%
%   Usage:   frf(sys1,'color/linetype/linewidth/marker/markersize',sys2,'color/linetype/linewidth/marker/markersize',...
%            ...{fmin,fmax,magmin,magmax,phasemin,phasemax,wraponoff,type})
%
%   Inputs:  You can use custom colors, e.g. lightgrey, pink, etc. (see line ~315)
%            example strings: 'lightgrey--1.5o12'    lightgrey dashed line with circle marker of size 12 
%                             'orange'               orange solid line, no marker
%                             'b*10'                 blue markers size 10, no line
%            All elements in the options-struct are optional
%            fmin/fmax in Hz
%            magmin/magmax in dB
%            phasemin/phasemax in degrees
%            wraponoff: [ 'on' | 'off' (default) ]
%            type: [ 'mag' | 'phase' | 'magphase' (default) ]
%
%   Outputs: Plots the frf in the current figure.
%            Note that the xticklabels might overlap the xlabel in the
%            figure. In an exported pdf file they will be correct.
%
%   See also FRFMAG, FRFPHASE, SETPLOT, SAVER

%   Changes:
%   20110803 - Initial version (Rob Hoogendijk)
%	20110914 - Changed rotation and alignment of Out: i labels (Michael Ronde)
%   20110916 - Small bugfix (Out: i labels for every system) (Michael Ronde)
% 	20111013 - Added axes handles as optional output (Michael Ronde)
%   20111016 - Bugfix for panning and zooming of MIMO plots (Michael Ronde)
%   20120203 - Cleanup and add linewidth/markersize option (Rob Hoogendijk/Michael Ronde)

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

function varargout = frf(varargin)
%% read inputs
nsys=0; %couter for number of systems
defaultcolors='bgrcmkybgrcmkybgrcmkybgrcmkybgrcmkybgrcmky';
for ii=1:length(varargin)
    inputii=varargin{ii};
    %detect systems, set default styles
    if issystem(inputii)
        nsys=nsys+1;
        systems{nsys}.lti=inputii;
        systems{nsys}.noutputs=size(systems{nsys}.lti,1);
        systems{nsys}.ninputs=size(systems{nsys}.lti,2);
        systems{nsys}.linetype='-'; %default style is only a line
        systems{nsys}.color=defaultcolors(nsys); %default linewidth
        systems{nsys}.linewidth=1.5;
        systems{nsys}.markersize=10;    
    end
    %detect formatting
    if ischar(inputii)
        systems{nsys}.style=inputii;
        temp = cst_plotstr_to_struct(inputii);       
        if(~isempty(temp.color)),      systems{nsys}.color     = temp.color;            end
        if(~isempty(temp.linetype)),   systems{nsys}.linetype  = temp.linetype;         end
        if(~isempty(temp.linewidth)),  systems{nsys}.linewidth = temp.linewidth;        end
        if(~isempty(temp.markersize)), systems{nsys}.linetype  = [systems{nsys}.linetype temp.marker]; end
    end
    %detect options
    if iscell(inputii)
        options=inputii;
    end
end
%try to process options (if not given set default values)
try fmin     = options{1};   catch, fmin=[];             end;
try fmax     = options{2};   catch, fmax=[];             end;
try magmin   = options{3};   catch, magmin=[];           end;
try magmax   = options{4};   catch, magmax=[];           end;
try phasemin = options{5};   catch, phasemin=[];         end;
try phasemax = options{6};   catch, phasemax=[];         end;
try wrapping = options{7};   catch, wrapping='off';      end;
try plottype = options{8};   catch, plottype='magphase'; end;



%% create_axes_handles
noutputs=0;ninputs=0;%init
for ii=1:nsys %detect max system size
    maxnoutputs=max( noutputs , systems{ii}.noutputs );
    maxninputs=max( ninputs , systems{ii}.ninputs );
end
% determine plottype
switch plottype 
    case 'magphase', plotmag=1;plotphase=1;
    case 'mag',      plotmag=1;plotphase=0;
    case 'phase',    plotmag=0;plotphase=1; 
end;
%generate plot handles
udata=get(gcf,'UserData');
firstplot=1;
try %try to read handles from userdata of figure to enable subsequent plotting    
    handles=udata.handles;
    phasehandles=udata.phasehandles;
    maghandles=udata.maghandles;
    %todo: check size/type of plot and use this to check if its feasible to
    %plot in the same graph else error or maybe add subplots
catch    
    handles=zeros((plotmag+plotphase)*maxnoutputs,maxninputs);
    maghandles=zeros(maxnoutputs,maxninputs);
    phasehandles=zeros(maxnoutputs,maxninputs);
    for nn=1:maxnoutputs
        for mm=1:maxninputs
            if(plotmag && plotphase)
                handles(2*nn-1,mm)=subplot(2*maxnoutputs,maxninputs,(2*(nn-1))*maxninputs+mm);hold on;grid on;       
                maghandles(nn,mm)=handles(2*nn-1,mm);%mag
                handles(2*nn,mm)=subplot(2*maxnoutputs,maxninputs,(2*(nn-1)+1)*maxninputs+mm);hold on;grid on;
                phasehandles(nn,mm)=handles(2*nn,mm);%phase
            else
                handles(nn,mm)=subplot(maxnoutputs,maxninputs,(nn-1)*maxninputs+mm);hold on;grid on;
                if(plotmag)
                    maghandles(nn,mm)=handles(nn,mm);%mag
                    phasehandles=[];
                else
                    phasehandles(nn,mm)=handles(nn,mm);%phase
                    maghandles=[];
                end
            end            
        end        
    end
    udata.handles=handles;
    udata.maghandles=maghandles;
    udata.phasehandles=phasehandles;
    set(gcf,'UserData',udata);
end



%% plot_frf
%plotting
for ii=1:nsys
    sys=systems{ii};
    if(~isempty(fmin)&&~isempty(fmax))
        [sys.mag,sys.phase,sys.w]=bode(sys.lti,{2*pi*fmin 2*pi*fmax});
    else
        [sys.mag,sys.phase,sys.w]=bode(sys.lti);
    end
    sys.f=sys.w/2/pi;
    if(strcmp(wrapping,'on'));
        sys.phase=wrap(sys.phase);
    end
    for nn=1:sys.noutputs
        for mm=1:sys.ninputs            
            %magnitude plots
            if(plotmag)
                semilogx(handles((plotmag+plotphase)*nn-plotphase,mm),sys.f,db(squeeze(sys.mag(nn,mm,:))),sys.linetype,'color',sys.color,'linewidth',sys.linewidth,'markersize',sys.markersize);                                           
            end
            %phase plots
            if(plotphase)                
                semilogx(handles((plotmag+plotphase)*nn,mm),sys.f,squeeze(sys.phase(nn,mm,:)),sys.linetype,'color',sys.color,'linewidth',sys.linewidth,'markersize',sys.markersize);                                 
            end
        end
    end
end
set(handles(:),'xscale','log','box','on');


%axes limits settings
ha=handles(:);
for ii=1:length(ha)
    hc=get(ha(ii),'children');               
    for jj=1:length(hc)
        maxx(ii,jj)=max(get(hc(jj),'xdata'));
        minx(ii,jj)=min(get(hc(jj),'xdata'));
    end
end   
if(isempty(fmin)), fmin=min(min(minx)); end
if(isempty(fmax)), fmax=max(max(maxx)); end
set(ha,'xlim',[fmin fmax]);
%magnitude axis
ha=maghandles(:);
for ii=1:length(ha)
    hc=get(ha(ii),'children');               
    for jj=1:length(hc)
        maxy(ii,jj)=max(get(hc(jj),'ydata'));
        miny(ii,jj)=min(get(hc(jj),'ydata'));
    end
end
if(isempty(magmin)), magmin=min(min(miny)); end
if(isempty(magmax)), magmax=max(max(maxy)); end
set(ha,'ylim',[magmin magmax]);
%phase axis
ha=phasehandles(:);
for ii=1:length(ha)
    hc=get(ha(ii),'children');               
    for jj=1:length(hc)
        maxy(ii,jj)=max(get(hc(jj),'ydata'));
        miny(ii,jj)=min(get(hc(jj),'ydata'));
    end
end
if(isempty(phasemin)), phasemin=min(min(miny)); end
if(isempty(phasemax)), phasemax=max(max(maxy)); end
set(ha,'ylim',[phasemin phasemax]);



%% add labels, ticks etc.
%detect mimo or siso
if(maxnoutputs>1||maxninputs>1)
    mimo=1;
else
    mimo=0;
end
%set ticks
%remove x ticks
set(handles(1:end-1,:),'XTickLabel',{});
%remove y ticks 
set(handles(:,2:end),'YTickLabel',{});
%set ticks for phase plots
set(phasehandles,'ytick',-36000:90:36000);  

%set labels
magtag='Magnitude [dB]';
phasetag='Phase[deg.]';
freqtag='Frequency [Hz]';
if(mimo)
    % one x and y label for mimo systems
    if(isempty(findall(gcf,'type','axes','visible','off')))%if invisible axis does not exist
        ax=axes('position',[0.1,0.1,1,1],'visible','off');
        xlab=get(ax,'xlabel');       ylab=get(ax,'ylabel');
        if(plotmag && plotphase),set(ylab,'string',[magtag ', ' phasetag],'visible','on');end
        if(plotmag && ~plotphase),set(ylab,'string',magtag,'visible','on');end
        if(~plotmag && plotphase),set(ylab,'string',phasetag,'visible','on');end
        set(xlab,'string',freqtag,'visible','on');
        % Change order of the axes, otherwise axes for global labels is in front,
        % i.e. other axes cannot be panned/zoomed etc
        set(gcf,'Children',flipud(get(gcf,'Children')))
    end
else   
   %set x handles to freq 
   xhandles=get(handles(end,:),'XLabel');  
   for ii=1:length(xhandles)
       if(length(xhandles)==1)%then its not a cell array
           set(xhandles,'string',freqtag);
       else%it is a cell array
           set(xhandles{ii},'string',freqtag);
       end              
   end
   %set y handles to mag or phase
   set(get(maghandles,'ylabel'),'string',magtag);
   set(get(phasehandles,'ylabel'),'string',phasetag); 
end

%in/out labels
if(mimo)
    row1hnd=handles(1,:);
    for ii=1:length(row1hnd)
        title(row1hnd(ii),['In:' num2str(ii)],'fontsize',8);
    end
    colendhnd=handles(:,end);
    for ii=1:length(colendhnd)        
        outputnumber=ceil(ii/(plotmag+plotphase));
        ylabel(colendhnd(ii),['Out:' num2str(outputnumber)],'fontsize',8,'tag','right');
    end
end



%% change_figure_settings
set(gcf,'color',[0.57 0.76 0.95]); %fancy blue

%axis positions 
if exist('setplot','file')==2
    setplot([], [], {0.5,1.0,[],1.1,1.1,[]});
else
    disp('Install setplot.m for optimal viewing of your frf.')
end

%linking axis
for ii=1:size(handles,2)%link x axes
    eval(['global link_fig' num2str(gcf) '_xaxis_' num2str(ii)]);
    eval(['link_fig' num2str(gcf) '_xaxis_' num2str(ii) '=linkprop(handles(:,ii),''XLim'' );']);
end
for ii=1:size(handles,1)%link y axes
    eval(['global link_fig' num2str(gcf) '_yaxis_' num2str(ii)]);
    eval(['link_fig' num2str(gcf) '_yaxis_' num2str(ii) '=linkprop(handles(ii,:),''YLim'' );']);
end

%% set outputs
varargout{1}=handles;
end

%% private functions
function phase=wrap(phase)
phase = phase-(round(phase/360)*360);
end

function plotobj = cst_plotstr_to_struct(plotstr) 
%          b     blue          \.    point             -     solid 
%          g     green         o     circle             :     dotted 
%          r     red           x     x-mark             -.    dashdot 
%          c     cyan          +     plus               --    dashed 
%          m     magenta       *     star             (none)  no line 
%          y     yellow        s     square 
%          k     black         d     diamond 
%          w     white         v     triangle (down) 
%                              ^     triangle (up) 
%                              <     triangle (left) 
%                              >     triangle (right) 
%                              p     pentagram 
%                              h     hexagram 
plotobj.color = '';
plotobj.linetype = '';
plotobj.linewidth = '';
plotobj.marker = '';
plotobj.markersize = '';

% Custom colors definitions
    custom= {...
        'lightgrey', [0.80 0.80 0.80];
        'midgrey',   [0.60 0.60 0.60];
        'darkgrey',  [0.40 0.40 0.40];
        'orange',    [1.00 0.50 0.00];
        'pink',      [1.00 0.50 0.60];
        'purple',    [0.80 0.00 1.00];
        'darkgreen', [0.00 0.70 0.00]; };

    %% Color options
    % Check if custom color option is found
    for j=1:size(custom,1)
        k=regexp(plotstr,custom{j,1}, 'once');
        if ~isempty(k)
            plotobj.color = custom{j,2};   
            plotstr = strrep(plotstr,custom{j,1},'');
        end
    end
    %else look for matlab color
    if(isempty(plotobj.color))
         plotobj.color = regexp(plotstr,'[rgbcymkw]','match','once');
    end    
    
    %%linestyle
    plotobj.linetype=regexp(plotstr,'(--)|(-\.)|(:)|(-)','match','once'); 
    
    %%marker
    [plotobj.marker markerindex]=regexp(plotstr,'[ox+*sd<>v^ph]','match','once');        
    % for dot marker use \.   
    temp=regexp(plotstr,'\\.');
    if(~isempty(temp))
        plotobj.marker='.';
        markerindex=temp;
    end
    
    %%numeric entries
    [nums, index] = regexp(plotstr,'\d+\.*\d*','match');
    for ii=1:length(nums)
       if(index(ii)>markerindex)
           plotobj.markersize=str2double(nums{ii});
       else
           plotobj.linewidth=str2double(nums{ii});
       end
    end   
end  
 
 

 
 
