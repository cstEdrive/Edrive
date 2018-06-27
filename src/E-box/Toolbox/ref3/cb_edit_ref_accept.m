
global irow
global ref_part

if irow>1

% if repeat add extra row

  hdl_edit_ref_repeat=findobj('Tag','tag_edit_ref_repeat');
  irepeat=get(hdl_edit_ref_repeat,'Value');
  if irepeat
    ref_part=[ref_part; 0.0 -1.0 0.0 0.0 0.0 0.0];
    save ref_part.mat ref_part;
  end
  save r3gbak irow refdesign irepeat;
end

close(gcf);


