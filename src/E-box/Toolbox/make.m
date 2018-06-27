% make file to install toolboxes

disp('Would you like to install the TU/e toolboxes?')
disp('0 - no (default)')
disp('1 - yes')

install_tue = input('? \n');

if(~isempty(install_tue))
    if install_tue == 1
        current_tbx_dir = cd;
        tbxpath = toolboxdir('');
        
        % FRFFit
        eval(['!cp -R frffit/ ',tbxpath])
        addpath([tbxpath,'/frffit/'])
        
        % Ref3ma
        eval(['!cp -R ref3ma/ ',tbxpath])
        addpath([tbxpath,'/ref3ma/']) 
        cd(toolboxdir('ref3ma'))
        mex ref3.c        
        mex ref3b.c
        cd(current_tbx_dir)        
        
        % ShapeIt
%         mkdir([toolboxdir(''),'/shapeit']);
        eval(['!cp -R shapeit/ ',tbxpath])
        cd(toolboxdir('shapeit'))
        shapeit_setup_runonce
        cd(current_tbx_dir)
        
        savepath
        rehash toolboxcache
    else
        disp('TU/e toolboxes not installed')
    end
end