%SETPLOT   Set figure size, text size, margins axis sizes etc   
%   
%   Usage:   setplot(fig, figsize, {top, bottom, rows, left, right, cols}, fontsize, fontname)  
%            to use defaults on current figure, run without arguments       
%
%   Inputs:  fig = figure handle   
%            figsize = [figwidth figheight] in cm  
%                          *1       *2  (see figure below)  
%            margins = {top, bottom, rows, left, right, cols} in cm  
%                       *m1  *m2     *m3   *m4   *m5    *m6  (see figure below)
%                       note: leave margin empty [] to use optimal values
%                             e.g. {[],[],[],1.5} only sets left margin to 1.5 cm    
%            fontsize = size of the font                
%            fontname = type of fontfile:  
%            _____________________________________________  _ _ _ _ _ __  
%           |        __________         __________        | _ _ _*m1   ^  
%           |   y   |          |       |          |       |            |  
%           |   l   | subplot1 |       | subplot2 |       |            .  
%           |   b   |          |       |          |       |            |   
%           |   l   |__________|       |__________|       | _ _ _      .                                                      
%           |                                             |      *m3   |  
%           |        __________         __________        | _ _ _     *2  
%           |   y   |          |       |          |       |            |   
%           |   l   | subplot3 |       | subplot4 |       |            .  
%           |   b   |          |       |          |       |            |   
%           |   l   |__________|       |__________|       | _ _ _      .   
%           |          xlabel             xlabel          |      *m2   |   
%           |_____________________________________________| _ _ _ _ _ _v_   
%           
%           |       |          |       |          |       |  
%           |<-*m4->|          |<-*m6->|          |<-*m5->|  
%           |<-------------- *1 ------------------------->|  
%      
%   
%   Outputs: none
%   
%   See also SAVER. 

%   Changes: 
%   20111025 - Bugfix: added drawnow command
%   20110804 - Added legend support and improved inputs
%   20110803 - Initial version (Rob Hoogendijk)
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

function setplot(fig, figsize, margins, fontsize, fontname)  
drawnow;%to make sure all figures are displayed
%% process inputs
%figure handle
if (nargin<1 || isempty(fig) )  
    fig=gcf;  
end  
set(fig,'PaperUnits','centimeters');  
set(fig,'Units','centimeters');  
%figure size
if (nargin<2 || isempty(figsize) )  
    figpos=get(fig,'position');  
    figsize=[figpos(3) figpos(4)];  
end
%margins
try margins{1}; catch margins={};end;
try topmargin   =margins{1}; catch topmargin=[];    end; 
try bottommargin=margins{2}; catch bottommargin=[]; end; 
try rowsmargin  =margins{3}; catch rowsmargin=[];   end; 
try leftmargin  =margins{4}; catch leftmargin=[];   end; 
try rightmargin =margins{5}; catch rightmargin=[];  end; 
try colsmargin  =margins{6}; catch colsmargin=[];   end;     
%fontsize
if (nargin<4 || isempty(fontsize) )  
    fontsize=10;  
end
%fontname
if (nargin<5 || isempty(fontname) )
    fontname='arial';  
end  
  
%% general settings  
%get handles to all axes and text  
gtxt    =   findall(fig,'type','text','visible','on');  
gax     =   findall(fig,'type','axes','visible','on','Tag','');  
nax=length(gax);  
set(gax,'units','centimeters');  
  
% set figure size  
currentpos = get(fig,'Position');  
set(fig,'Position',[currentpos(1:2) figsize]);    
  
%set fontsizes  
set(gtxt,'FontSize',fontsize,'FontName',fontname);  
%set(gtxt,'interpreter','LaTex');
set(gax,'FontSize',0.8*fontsize,'FontName',fontname);  
fontspace=fontsize*(1/72)*2.54; %in cm  
  
%% get info from figure  
%get all axis positions  
for ii=1:nax  
    posxy(ii,:)=round(get(gax(ii),'position')*100)/100;  
end  
%put axes handles in subplots(nrows,ncols)  
xval=unique(sort(posxy(:,1)));  yval=flipud(unique(sort(posxy(:,2))));  
ncols=length(xval);             nrows=length(yval);  
%get axis subplot indices  
for jj=1:nax  
    for ii=1:nrows  
        if(posxy(jj,2)==yval(ii)), row=ii; end  
    end  
    for ii=1:ncols  
        if(posxy(jj,1)==xval(ii)) col=ii; end  
    end  
    subplots(row,col)=gax(jj);  
end    
% store relevant handles in arrays, set units to cm   
for ii=1:nrows  
    for jj=1:ncols   
        ax=subplots(ii,jj);  
        %labels  
        xlab(ii,jj)=get(ax,'xlabel'); 
        hasxlab(ii,jj)=~isempty(get(xlab(ii,jj),'string')); 
        set(xlab(ii,jj),'units','centimeters');  
        ylab(ii,jj)=get(ax,'ylabel'); 
        hasylab(ii,jj)=~isempty(get(ylab(ii,jj),'string')) && ~strcmp(get(ylab(ii,jj),'tag'),'right'); 
        set(ylab(ii,jj),'units','centimeters');  
        titl(ii,jj)=get(ax,'title');  
        hastitl(ii,jj)=~isempty(get(titl(ii,jj),'string')); 
        set(titl(ii,jj),'units','centimeters');  
        % ticks  
        hasxticklbls(ii,jj)=~isempty(get(ax,'xticklabel'));  
        hasyticklbls(ii,jj)=~isempty(get(ax,'yticklabel'));  
        % tightinset (excluding labels)  
        set(gtxt,'visible','off');  
        tis=get(ax,'tightinset');  
        tisleft(ii,jj)=tis(1); tisbottom(ii,jj)=tis(2); tisright(ii,jj)=tis(3); tistop(ii,jj)=tis(4);  
        set(gtxt,'visible','on');          
    end  
end  

%detect titles, labels, ticks
istitletop        = ~isequal( hastitl(1,:) , zeros(1,ncols) );
isxlabelinbetween = ~isequal( hasxlab(1:end-1,:) , zeros(nrows-1,ncols) );
isxticksinbetween = ~isequal( hasxticklbls(1:end-1,:) , zeros(nrows-1,ncols) ); 
isxlabelbottom    = ~isequal( hasxlab(end,:) , zeros(1,ncols) );
isylabelinbetween = ~isequal( hasylab(:,2:end) , zeros(nrows,ncols-1) );
isyticksinbetween = ~isequal( hasyticklbls(:,2:end) , zeros(nrows,ncols-1) );
isylabelleft      = ~isequal( hasylab(:,1) , zeros(nrows,1) );

% compute maximum space between the plots
mtistop    = max(tistop(1,:));
mtisbottom = max(tisbottom(end,:));
mtisleft   = max(tisleft(:,1));
mtisright  = max(tisright(:,end));
mtiscols   = max(max(tisleft(:,2:end)))+max(max(tisright(:,1:end-1))); if(isempty(mtiscols)),mtiscols=0;end;     
mtisrows   = max(max(tisbottom(1:end-1,:)))+max(max(tistop(2:end,:))); if(isempty(mtisrows)),mtisrows=0;end;         

%determine margins if not given in inputs
if(isempty(colsmargin)),   colsmargin=mtiscols+isylabelinbetween*fontspace;                 end;
if(isempty(rowsmargin)),   rowsmargin=max((mtisrows+isxlabelinbetween*fontspace)-0.1, 0.1); end;
if(isempty(leftmargin)),   leftmargin=mtisleft+isylabelleft*fontspace+0.1;                  end;
if(isempty(bottommargin)), bottommargin=mtisbottom+isxlabelbottom*fontspace;                end;
if(isempty(rightmargin)),  rightmargin=mtisright;                                           end;
if(isempty(topmargin)),    topmargin=mtistop+istitletop*fontspace;                          end;
%if no labels and no ticks in between, use same spacing  
if( ~isxlabelinbetween && ~isxlabelinbetween && isxticksinbetween && isyticksinbetween)  
    colsmargin=max(colsmargin,rowsmargin);
    rowsmargin=max(colsmargin,rowsmargin);  
end
%if left and right margin not user defined, use same space left and right
if(leftmargin==mtisleft+isylabelleft*fontspace+0.1 && rightmargin==mtisright)
    leftmargin=max(leftmargin,rightmargin);
    rightmargin=max(leftmargin,rightmargin);  
end 

%determine size of axes
axwidth = (figsize(1)-rightmargin-leftmargin  -(ncols-1)*colsmargin)/ncols;  
axheight= (figsize(2)-topmargin  -bottommargin-(nrows-1)*rowsmargin)/nrows;  
  
%determine desired axes positions  
axcolpos=leftmargin+([1:ncols]-1)*(axwidth+colsmargin);  
axrowpos=fliplr(bottommargin+([1:nrows]-1)*(axheight+rowsmargin));  
  
%% set axes positions, labels  
for ii=1:nrows  
    for jj=1:ncols  
        ax=subplots(ii,jj);          
        set(ax,'position',[axcolpos(jj),axrowpos(ii),axwidth,axheight]);       
        set(xlab(ii,jj),'position',[0.5*axwidth                                  -max(tisbottom(ii,:)) 0]);  
        if(strcmp(get(ylab(ii,jj),'tag'),'right'))%for labels on the right side set the tag of the label to 'right'
            set(ylab(ii,jj),'position',[axwidth+fontspace   0.5*axheight         0]);  
        else
            set(ylab(ii,jj),'position',[-max(tisleft(:,jj))-hasyticklbls(ii,jj)*0.1   0.5*axheight         0]);  
        end
        set(titl(ii,jj),'position',[0.5*axwidth                                   axheight             0]);  
    end  
end  
  
%% set invisible axis position to use one xlabel and one ylabel for allsubplots
ivax = findall(fig,'type','axes','visible','off');   
set(ivax,'units','centimeters','position',[axcolpos(1) axrowpos(end) axcolpos(end)+axwidth-axcolpos(1) axrowpos(1)+axheight-axrowpos(end)]);  
  
  
%% set resize function to this function with the proper arguments  
eval([ 'set(' num2str(fig) ',''ResizeFcn'',''setplot(' num2str(fig) ',[], {' num2str(topmargin) ',' num2str(bottommargin) ',' num2str(rowsmargin) ',' num2str(leftmargin) ',' num2str(rightmargin) ',' num2str(colsmargin) '},' num2str(fontsize) ');'');' ]);  
%disp([ 'set(' num2str(fig) ',''ResizeFcn'',''setplot(' num2str(fig) ',[], {' num2str(topmargin) ',' num2str(bottommargin) ',' num2str(rowsmargin) ',' num2str(leftmargin) ',' num2str(rightmargin) ',' num2str(colsmargin) '},' num2str(fontsize) ',''' fontname ''');'');' ]);  

set(fig,'Units','pixels'); 
  
end %function   


