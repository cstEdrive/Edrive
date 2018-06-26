
clear refdesign;

global irow refdesign
global partexist

partexist=0;
irow=1;

% initialize refdesign matrix

% t0, te, x0, xe, vel, acc, jerk

refdesign(1,1)=0.0;
refdesign(1,2)=0.0;
refdesign(1,3)=0.0;
refdesign(1,4)=1.0;
refdesign(1,5)=0.5;
refdesign(1,6)=1.0;
refdesign(1,7)=10.0;

if exist('r3gbak.mat','file')
  load r3gbak;
end

R12_r3g_edit_ref;
if irow==1
  hdl_edit_ref_edit=findobj('Tag','tag_edit_ref_edit');
  set(hdl_edit_ref_edit,'Enable','off');
  hdl_edit_ref_save=findobj('Tag','tag_edit_ref_save');
  set(hdl_edit_ref_save,'Enable','off');
end
hdl_tag_edit_ref_status=findobj('Tag','tag_edit_ref_status');
set(hdl_tag_edit_ref_status,'String','Waiting for button to be pressed');
show_ref;
if irepeat
  hdl_edit_ref_repeat=findobj('Tag','tag_edit_ref_repeat');
  set(hdl_edit_ref_repeat,'Value',1);
end
