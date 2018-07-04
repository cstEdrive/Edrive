% make file to install toolboxes

install_toolbox = input('Would you like to install the TU/e toolboxes? y/n [y]: ', 's');

if strcmp(install_toolbox,'y') || strcmp(install_toolbox,'')
    current_tbx_dir = pwd;
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
elseif strcmp(install_toolbox,'n')
    disp('TU/e toolboxes not installed')
end