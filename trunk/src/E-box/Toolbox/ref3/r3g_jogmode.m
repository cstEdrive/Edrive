function fig = r3g_jogmode()
% This is the machine-generated representation of a Handle Graphics object
% and its children.  Note that handle values may change when these objects
% are re-created. This may cause problems with any callbacks written to
% depend on the value of the handle at the time the object was saved.
% This problem is solved by saving the output as a FIG-file.
%
% To reopen this object, just type the name of the M-file at the MATLAB
% prompt. The M-file and its associated MAT-file must be on your path.
% 
% NOTE: certain newer features in MATLAB may not have been saved in this
% M-file due to limitations of this format, which has been superseded by
% FIG-files.  Figures which have been annotated using the plot editor tools
% are incompatible with the M-file/MAT-file format, and should be saved as
% FIG-files.

load r3g_jogmode

h0 = figure('Color',[0.8 0.8 0.8], ...
	'Colormap',mat0, ...
	'FileName','D:\Ttt2\r3g_jogmode.m', ...
	'PaperPosition',[18 180 576 432], ...
	'PaperUnits','points', ...
	'Position',[322 306 444 225], ...
	'Tag','Fig1', ...
	'ToolBar','none');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'ListboxTop',0, ...
	'Position',[23.25 133.5 288 18], ...
	'String','In jogmode, a constant velocity reference can be designed.', ...
	'Style','text', ...
	'Tag','StaticText1');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[1 1 1], ...
	'Callback','cb_jogmode_input', ...
	'ListboxTop',0, ...
	'Position',[85.5 111 71.25 15.75], ...
	'String','0', ...
	'Style','edit', ...
	'Tag','tag_jogmode_t0');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'ListboxTop',0, ...
	'Position',[55.5 109.5 22.5 14.25], ...
	'String','t0', ...
	'Style','text', ...
	'Tag','tag_jogmode_text_t0');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'ListboxTop',0, ...
	'Position',[52.5 50.25 30.75 15.75], ...
	'String','vel', ...
	'Style','text', ...
	'Tag','tag_jogmode_text_vel');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[1 1 1], ...
	'Callback','cb_jogmode_input', ...
	'ListboxTop',0, ...
	'Position',[85.5 53.25 72.75 15.75], ...
	'String','1', ...
	'Style','edit', ...
	'Tag','tag_jogmode_vel');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[0.752941176470588 0.752941176470588 0.752941176470588], ...
	'Callback','cb_jogmode_accept', ...
	'ListboxTop',0, ...
	'Position',[225 14.25 48 21], ...
	'String','Accept', ...
	'Tag','tag_jogmode_accept');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[1 1 1], ...
	'Callback','cb_jogmode_input', ...
	'ListboxTop',0, ...
	'Position',[85.5 81.75 72 15.75], ...
	'String','0', ...
	'Style','edit', ...
	'Tag','tag_jogmode_x0');
h1 = uicontrol('Parent',h0, ...
	'Units','points', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'ListboxTop',0, ...
	'Position',[50.25 79.5 32.25 16.5], ...
	'String','x0', ...
	'Style','text', ...
	'Tag','tag_jogmode_text_x0');
if nargout > 0, fig = h0; end