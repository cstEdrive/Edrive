disp('%%%%%%%%%%%%% Installing EtherCAT (E-box and Beckhoff) target software %%%%%%%%%%%%%%%%')
cd ectarget/
% mex EtherCAT target s-functions
make
% move mexglx-files to /usr/local/matlab*/rtw/c/ectarget/sfunctions/
eval( ['!sudo mv -f *.mex* ' matlabroot '/rtw/c/ectarget/sfunctions/']);
cd ..
cd ..

% copy motion target to /usr/local/matlab*/rtw/c
eval( ['!sudo cp -rf ectarget ' matlabroot '/rtw/c']);
disp('ectarget installed')

% For matlab 2010a and newer copy alternative target makefile (ec_unix.tmf)
if verLessThan('matlab','7.13')
    % Check for version of RTW
    if ~verLessThan('rtw','7.5')
        disp('Copying alternative makefile')        
        eval(['!cp  ../Dev/Michael/matlab2010a/ectarget/ec_unix.tmf ',matlabroot,'/rtw/c/ectarget/ec_unix.tmf'])
    end
else
    % Check for version of simulinkcoder, i.e. new name for RTW
    if ~verLessThan('simulinkcoder','7.5')
        disp('Copying alternative makefile')
        eval(['!cp  ../Dev/Michael/matlab2010a/ectarget/ec_unix.tmf ',matlabroot,'/rtw/c/ectarget/ec_unix.tmf'])
    end
end