% make_all_clean
% makefile which cleans all source directories from binary files

% Find and delete *_et_rtw and slprj directories
% Linux commands rm -f (i.e. remove recursive force)
!find -name '*_et_rtw' | xargs rm -rf
!find -name 'slprj' | xargs rm -rf
