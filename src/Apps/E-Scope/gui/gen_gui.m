
%           'tag_gen_chan_ampl', 56,...
%           'tag_gen_chan_ampl_2', 57,...
%           'tag_gen_chan_freq', 58,...
%           'tag_gen_chan_freq_2', 59,...
%           'tag_gen_chan_time', 60,...
%           'tag_gen_chan_time_2', 61,...
%           'tag_gen_chan_ampl_focus', 64,...
%           'tag_gen_chan_freq_focus', 65,...
%           'tag_gen_link', 66,...
%           'tag_gen_power', 67,...
%           'tag_gen_exit', 68);

function gen_gui()

global g def;

g=scope_globals;

def=scope_defines;

g.hgui(2)=figure;
set(g.hgui(2),'Position',[def.XGUI, def.YGUI, def.GUIWIDTH, def.GUIHEIGHT]);
set(g.hgui(2),'MenuBar','none');
set(g.hgui(2),'NumberTitle','Off');
set(g.hgui(2),'Name','Signal Generator');

% left and right column
xleft=def.XGUI+def.XMARGIN;
xright=def.XGUI+def.XMARGIN+2*def.POPUPWIDTH-2*def.XMARGIN;
% top row
ytop=def.YGUI+def.GUIHEIGHT-def.YMARGIN;

% title Chan settings
x=xleft;
y=ytop;
w=def.POPUPWIDTH;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_gen_chan_title)=uicontrol_gen_chan_title(pos);

% signal popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_gen_chan_signal)=uicontrol_gen_chan_signal(pos);

% enable button
x=x+def.POPUPWIDTH;
w=def.BUTTONHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_chan_signal_enable)=uicontrol_gen_chan_signal_enable(pos);

% offset slider
x=xleft;
y=y-def.SLIDERHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.SLIDERWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_chan_offset)=uicontrol_gen_chan_offset(pos);

% zeroise button
x=x+def.SLIDERWIDTH;
w=def.BUTTONHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_chan_offset_zeroise)=uicontrol_gen_chan_offset_zeroise(pos);

% offset edit
x=xleft;
y=y-def.EDITHEIGHT;
w=def.POPUPWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_chan_offset_2)=uicontrol_gen_chan_offset_2(pos);

% amplitude slider
x=xleft;
y=y-def.SLIDERHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.SLIDERWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_gen_chan_ampl)=uicontrol_gen_chan_ampl(pos);

% amplitude edit
x=xleft;
y=y-def.EDITHEIGHT;
w=def.POPUPWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_gen_chan_ampl_2)=uicontrol_gen_chan_ampl_2(pos);

% sampling rate popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_meas_rate)=uicontrol_meas_rate(pos);

% aa button
%x=x+def.POPUPWIDTH;
%w=def.BUTTONHEIGHT;
%pos=[x, y, w, h];
%g.handles(def.tag_meas_aa)=uicontrol_meas_aa(pos);

% frame length popup
x=xleft;
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_meas_frame)=uicontrol_meas_frame(pos);

ybak=y; % store y to align with trigger settings later on

% windowing popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_meas_windowing)=uicontrol_meas_windowing(pos);

% number of frames popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_meas_nframes)=uicontrol_meas_nframes(pos);

% acquisition time display
y=y-def.EDITHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_meas_time)=uicontrol_meas_time(pos);

% current frame display
y=y-def.EDITHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_meas_current)=uicontrol_meas_current(pos);



% save last frame button
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_save_last)=uicontrol_save_last(pos);

% signal generator button
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_generator)=uicontrol_generator(pos);



% overload leds for adc's

% title overload monitor
y=y-def.TITLEHEIGHT-def.YMARGIN;
pos=[x, y, w, h];
g.handles(def.tag_overload_title)=uicontrol_overload_title(pos);

y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH/4;
h=def.POPUPWIDTH/10;
pos=[x, y, w, h];
g.handles(def.tag_led_adc0)=uicontrol_led_adc0(pos);

x=x+def.POPUPWIDTH/4;
pos=[x, y, w, h];
g.handles(def.tag_led_adc1)=uicontrol_led_adc1(pos);

x=x+def.POPUPWIDTH/4;
pos=[x, y, w, h];
g.handles(def.tag_led_adc2)=uicontrol_led_adc2(pos);

x=x+def.POPUPWIDTH/4;
pos=[x, y, w, h];
g.handles(def.tag_led_adc3)=uicontrol_led_adc3(pos);



% inst button
x=xleft;
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH/1.5;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_inst)=uicontrol_inst(pos);

% avg button
x=x+def.POPUPWIDTH/1.5;
pos=[x, y, w, h];
g.handles(def.tag_avg)=uicontrol_avg(pos);

% stop button
x=x+def.POPUPWIDTH/1.5;
pos=[x, y, w, h];
g.handles(def.tag_stop)=uicontrol_stop(pos);

% exit button
x=x+def.POPUPWIDTH;
pos=[x, y, w, h];
g.handles(def.tag_exit)=uicontrol_exit(pos);

% inst led
x=xleft;
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT;
w=def.POPUPWIDTH/1.5;
h=def.POPUPWIDTH/10;
pos=[x, y, w, h];
g.handles(def.tag_led_inst)=uicontrol_led_inst(pos);

% avg led
x=x+def.POPUPWIDTH/1.5;
pos=[x, y, w, h];
g.handles(def.tag_led_inst)=uicontrol_led_inst(pos);



% title copyright notice
x=xleft;
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=3*def.TITLEWIDTH;
h=def.TITLEHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_copyright_title)=uicontrol_copyright_title(pos);



% title Scope settings
x=xright;
y=ytop;
w=def.TITLEWIDTH;
h=def.TITLEHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_title)=uicontrol_scope_title(pos);

% id popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_id)=uicontrol_scope_id(pos);

% enable button
x=x+def.POPUPWIDTH;
w=def.POPUPWIDTH/2;
pos=[x, y, w, h];
g.handles(def.tag_scope_hold)=uicontrol_scope_hold(pos);

% type popup
x=xright;
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_type)=uicontrol_scope_type(pos);

% scale popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_scope_scale)=uicontrol_scope_scale(pos);

% freeze button
x=x+def.POPUPWIDTH;
w=def.POPUPWIDTH/2;
pos=[x, y, w, h];
g.handles(def.tag_scope_freeze)=uicontrol_scope_freeze(pos);

% adc0 checkbox
x=xright;
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.CHECKBOXWIDTH;
h=def.CHECKBOXHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_adc0)=uicontrol_scope_adc0(pos);

% enc0 checkbox
x=x+def.CHECKBOXWIDTH+def.XMARGIN;
pos=[x, y, w, h];
g.handles(def.tag_scope_enc0)=uicontrol_scope_enc0(pos);

% adc1 checkbox
x=xright;
y=y-def.CHECKBOXHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_adc1)=uicontrol_scope_adc1(pos);

% enc1 checkbox
x=x+def.CHECKBOXWIDTH+def.XMARGIN;
pos=[x, y, w, h];
g.handles(def.tag_scope_enc1)=uicontrol_scope_enc1(pos);

% adc2 checkbox
x=xright;
y=y-def.CHECKBOXHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_adc2)=uicontrol_scope_adc2(pos);

% enc2 checkbox
x=x+def.CHECKBOXWIDTH+def.XMARGIN;
pos=[x, y, w, h];
g.handles(def.tag_scope_enc2)=uicontrol_scope_enc2(pos);

% adc3 checkbox
x=xright;
y=y-def.CHECKBOXHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_scope_adc3)=uicontrol_scope_adc3(pos);

% enc3 checkbox
x=x+def.CHECKBOXWIDTH+def.XMARGIN;
pos=[x, y, w, h];
g.handles(def.tag_scope_enc3)=uicontrol_scope_enc3(pos);



% title Trigger settings
x=xright;
y=ybak;
w=def.TITLEWIDTH;
h=def.TITLEHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_trig_title)=uicontrol_trig_title(pos);

% trigger signal popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_trig_chan_signal)=uicontrol_trig_chan_signal(pos);

% trigger onoff button
x=x+def.POPUPWIDTH;
w=def.BUTTONHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_trig_onoff)=uicontrol_trig_onoff(pos);

% trigger level popup
x=xright;
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
w=def.POPUPWIDTH;
h=def.POPUPHEIGHT;
pos=[x, y, w, h];
g.handles(def.tag_trig_level)=uicontrol_trig_level(pos);

% trigger pre-roll popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_trig_preroll)=uicontrol_trig_preroll(pos);

% trigger low-pass popup
y=y-def.POPUPHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_trig_lowpass)=uicontrol_trig_lowpass(pos);

% load settings button
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_load_settings)=uicontrol_load_settings(pos);

% save settings button
y=y-def.BUTTONHEIGHT-def.TITLEHEIGHT-def.YMARGIN/4;
pos=[x, y, w, h];
g.handles(def.tag_save_settings)=uicontrol_save_settings(pos);

return





function h=uicontrol_gen_chan_title(pos)

global g;

data=scope_data;

data.style='text';

data.position=pos;

data.string='Channel';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_signal(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='analog in ch0';
data.string{2}='analog in ch1';
data.string{3}='analog in ch2';
data.string{4}='analog in ch3';

data.title='signal';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_offset(pos)

global g;

data=scope_data;

data.style='slider';

data.position=pos;

data.title='offset';

data.bounds=[-10.0, 10.0];
data.ticks=[10/100, 10/10];
data.value=0;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_offset_2(pos)

global g;

data=scope_data;

data.style='edit';

data.position=pos;

data.bounds=[-10.0, 10.0];
data.value=0;

data.displayonly=0;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_signal_enable(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title=' ';
data.string='on';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_offset_zeroise(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='0';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_ampl(pos)

global g;

data=scope_data;

data.style='slider';

data.position=pos;

data.title='amplitude';

data.bounds=[-10.0, 10.0];
data.ticks=[10/100, 10/10];
data.value=0;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_gen_chan_ampl_2(pos)

global g;

data=scope_data;

data.style='edit';

data.position=pos;

data.bounds=[-10.0, 10.0];
data.value=0;

data.displayonly=0;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_title(pos)

global g;

data=scope_data;

data.style='text';

data.position=pos;

data.string='Measurement';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_rate(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='8192 Hz';
data.string{2}='4096 Hz';
data.string{3}='2048 Hz';
data.string{4}='1024 Hz';
data.string{5}='512 Hz';
data.string{6}='256 Hz';
data.string{7}='128 Hz';

data.title='sampling rate';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_aa(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title='AA';
data.string='on';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_frame(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='64';
data.string{2}='128';
data.string{3}='256';
data.string{4}='512';
data.string{5}='1024';
data.string{6}='2048';
data.string{7}='4096';
data.string{8}='8192';

data.title='frame length';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_windowing(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='box car';
data.string{2}='von Hann';

data.title='windowing';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_nframes(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='infinite';
data.string{2}='1000';
data.string{3}='500';
data.string{4}='400';
data.string{5}='300';
data.string{6}='200';
data.string{7}='100';
data.string{8}='90';
data.string{9}='80';
data.string{10}='70';
data.string{11}='60';
data.string{12}='50';
data.string{13}='40';
data.string{14}='30';
data.string{15}='20';
data.string{16}='10';
data.string{17}='9';
data.string{18}='8';
data.string{19}='7';
data.string{20}='6';
data.string{21}='5';
data.string{22}='4';
data.string{23}='3';
data.string{24}='2';
data.string{25}='single shot';

data.title='number of frames';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_time(pos)

global g;

data=scope_data;

data.style='edit';

data.position=pos;

data.displayonly=1;
data.title='acquisition time';

data.value=0;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_meas_current(pos)

global g;

data=scope_data;

data.style='edit';

data.position=pos;

data.displayonly=1;
data.title='current frame';

data.value=1;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_save_last(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='save last frame';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_generator(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='signal generator';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_overload_title(pos)

global g;

data=scope_data;

data.style='text';

data.position=pos;

data.string='Overload';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_led_adc0(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title='adc0';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_led_adc1(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title='adc1';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_led_adc2(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title='adc2';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_led_adc3(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title='adc3';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_inst(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='inst';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_avg(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='avg';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_stop(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='stop';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_exit(pos)

global g;

data=scope_data;

data.style='pushbutton';

data.position=pos;

data.string='exit';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_led_inst(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title=' ';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_led_avg(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title=' ';
data.string=' ';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

set(h, 'Enable', 'off');

return





function h=uicontrol_scope_title(pos)

global g;

data=scope_data;

data.style='text';

data.position=pos;

data.string='Scope';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_id(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='scope 1';
data.string{2}='scope 2';

data.title='id';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_hold(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.string='hold';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_type(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='t';
data.string{2}='fft: modulus';

data.title='type';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_scale(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='linear';
data.string{2}='log';

data.title='scale';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_freeze(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.string='freeze';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_adc0(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='adc0';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_enc0(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='enc0';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_adc1(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='adc1';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_enc1(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='enc1';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_adc2(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='adc2';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_enc2(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='enc2';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_adc3(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='adc3';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_scope_enc3(pos)

global g def;

data=scope_data;

data.style='checkbox';

data.position=pos;

data.value=0;

data.string='enc3';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_title(pos)

global g;

data=scope_data;

data.style='text';

data.position=pos;

data.string='Trigger';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_chan_signal(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='analog in ch0';
data.string{2}='analog in ch1';
data.string{3}='analog in ch2';
data.string{4}='analog in ch3';

data.title='signal';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_onoff(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.title=' ';
data.string='on';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_level(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='100%';
data.string{2}='50%';
data.string{3}='25%';
data.string{4}='10%';
data.string{5}='5%';
data.string{6}='1%';
data.string{7}='0.1%';
data.string{8}='0%';
data.string{9}='-0.1%';
data.string{10}='-1%';
data.string{11}='-5%';
data.string{12}='-10%';
data.string{13}='-25%';
data.string{14}='-50%';
data.string{15}='-100%';

data.title='level';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_preroll(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='0%';
data.string{2}='5%';
data.string{3}='10%';
data.string{4}='20%';
data.string{5}='30%';
data.string{6}='40%';
data.string{7}='50%';

data.title='pre-roll';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_trig_lowpass(pos)

global g;

data=scope_data;

data.style='popupmenu';

data.position=pos;

data.string{1}='off';
data.string{2}='40%';
data.string{3}='30%';
data.string{4}='20%';
data.string{5}='10%';
data.string{6}='5%';
data.string{7}='0.5%';
data.string{8}='0.1%';

data.title='low-pass';

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_load_settings(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.string='load settings';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_save_settings(pos)

global g def;

data=scope_data;

data.style='togglebutton';

data.position=pos;

data.value=0;

data.string='save settings';
data.color=def.defaultcolor;

h=qs_uicontrol(g.hgui(2),data);

return





function h=uicontrol_copyright_title(pos)

global g def;

data=scope_data;

data.style='text';

data.position=pos;

data.string='(c) Marinus J.G. van de Molengraft, 2001 - 2011';

h=qs_uicontrol(g.hgui(2),data);

set(h, 'ForegroundColor', 0.7*def.defaultcolor);

return





function h=qs_uicontrol(hgui,data)

global def;

% create control
h=uicontrol(hgui);

switch data.style
case {'popupmenu', 'slider', 'togglebutton', 'edit'}
    if ~( strcmp(data.style,'edit') & data.displayonly==0 )
%   	set title above control
	h2=uicontrol(hgui);
	set(h2,'Style','text');
	set(h2,'HorizontalAlignment','left');
	set(h2,'BackgroundColor',[0.8, 0.8, 0.8]);
	set(h2,'String',data.title);
    end
end

% set style
set(h,'Style',data.style);

switch data.style
case 'popupmenu'
    set(h,'String',data.string);
    pos=[data.position(1), data.position(2)+def.POPUPHEIGHT, def.TITLEWIDTH, def.TITLEHEIGHT];
    set(h2,'Position',pos);    
case 'slider'
    set(h,'Min',data.bounds(1));
    set(h,'Max',data.bounds(2));
    set(h,'SliderStep',data.ticks);
    set(h,'Value',data.value);
    pos=[data.position(1), data.position(2)+def.SLIDERHEIGHT, def.TITLEWIDTH, def.TITLEHEIGHT];
    set(h2,'Position',pos);    
case 'edit'
    if data.displayonly
	set(h,'Enable','Off');
	set(h,'FontWeight','Bold');
	set(h,'String',num2str(data.value));
        pos=[data.position(1), data.position(2)+def.EDITHEIGHT, def.TITLEWIDTH, def.TITLEHEIGHT];
	set(h2,'Position',pos);    
    else
        set(h,'Min',data.bounds(1));
	set(h,'Max',data.bounds(2));
	set(h,'String',num2str(data.value));
    end
case 'text'
    set(h,'HorizontalAlignment','left');
    set(h,'BackgroundColor',[0.8, 0.8, 0.8]);
    set(h,'FontWeight','Bold');
    set(h,'String',data.string);
case 'togglebutton'
    set(h,'BackgroundColor',data.color);
    set(h,'Value',data.value);
    set(h,'String',data.string);
    pos=[data.position(1), data.position(2)+def.BUTTONHEIGHT, def.TITLEWIDTH, def.TITLEHEIGHT];
    set(h2,'Position',pos);     
case 'pushbutton'
    set(h,'Value',data.value);
    set(h,'String',data.string);
case 'checkbox'
    set(h,'Value',data.value);
    set(h,'String',data.string);
end

% set position control
set(h,'Position',data.position);

return
