%ADD_HEADER   Adds standard header to  .m or .c file 
%   
%   Usage:   add_header('filename.m') or add_header('filename.c')
%    
%   Inputs:  filename
%   
%   Outputs: none
%
%   Changes: 
%   20110718 - Initial version (Michael Ronde)
%   20110722 - Author, phone, room and email are variable (Michael Ronde)
%   20110804 - Made directory independent, Matlab style layout, input personal info (Rob Hoogendijk)
%   20111122 - Added windows compatibility (Rob Hoogendijk)
%
%   Michael Ronde (2011-07-18)
%   ________________________________
%   Eindhoven University of Technology
%   Dept. of Mechanical Engineering
%   Control Systems Technology group
%   PO Box 513, WH -1.127
%   5600 MB Eindhoven, NL
%   T +31 (0)40 247 2798
%   F +31 (0)40 246 1418
%   E m.j.c.ronde@tue.nl 

function add_header(filename)  

% exit if there is no such file
if(exist(filename,'file')~=2),error('No such file. Check filename and current directory.');end;

% load personal data, create if not available
ahdir = strrep(which('add_header.m'),'add_header.m',''); 
if(exist([ahdir,'personal_data.mat'])), load([ahdir,'personal_data.mat'])
else
    disp('This is the first time you use add_header, fill in your personalia')
    name = input('name:','s');
    phone = input('phone:','s');
    room = input('room number:','s');
    email = input('email:','s');
    eval(['save ' ahdir 'personal_data.mat name phone room email'] )
end
 
datevec = clock;  
year    = num2str(datevec(1));  
month   = num2str(datevec(2),'%2.2d');  
day     = num2str(datevec(3),'%2.2d');  
  
dir         =   strrep(which(filename),filename,''); 
fn          =   regexp(filename, '\.', 'split');  
filename    =   char(fn(1));  
extension   =   char(fn(2));  
  
if strcmpi(extension,'m')  
    comment_string = '%';  
    language_c = 0;  
elseif strcmpi(extension,'c')  
    comment_string          = ' *';  
    comment_string_begin    = '/*';  
    comment_string_end      = ' */';  
    language_c = 1;  
else  
    disp('Unsupported extension')  
end  
  
[fid_new, message] = fopen(strcat(dir,filename,'_header.',extension),'w');  
  
if language_c   
    fprintf(fid_new,'%s \n',comment_string_begin);  
end  
fprintf(fid_new,'%s%s   \n',comment_string, upper(filename));  
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   Usage:   \n',comment_string);  
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   Inputs:  \n',comment_string);  
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   Outputs: \n',comment_string);  
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   See also \n',comment_string);  
% empty newline to split help and changelog 
fprintf(fid_new,'\n');  
fprintf(fid_new,'%s   References: \n',comment_string);  
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   Changes: \n',comment_string);  
fprintf(fid_new,'%s   %s%s%s - Initial version (%s) \n',comment_string,year,month,day,name);  
fprintf(fid_new,'%s   \n',comment_string); 
fprintf(fid_new,'%s   \n',comment_string);  
fprintf(fid_new,'%s   %s (%s-%s-%s) \n',comment_string,name,year,month,day);  
fprintf(fid_new,'%s   ________________________________\n',comment_string);  
fprintf(fid_new,'%s   Eindhoven University of Technology\n',comment_string);  
fprintf(fid_new,'%s   Dept. of Mechanical Engineering\n',comment_string);  
fprintf(fid_new,'%s   Control Systems Technology group\n',comment_string);  
fprintf(fid_new,'%s   PO Box 513, %s\n',comment_string,room);  
fprintf(fid_new,'%s   5600 MB Eindhoven, NL\n',comment_string);  
fprintf(fid_new,'%s   T +31 (0)40 247 %s\n',comment_string,phone);  
fprintf(fid_new,'%s   F +31 (0)40 246 1418\n',comment_string);  
fprintf(fid_new,'%s   E %s\n',comment_string,email);  
  
%End comment string  
if language_c   
    fprintf(fid_new,'%s \n \n',comment_string_end);  
else   
    fprintf(fid_new,'\n');  
end  
 
% Original file 
[fid_org, message] = fopen([dir,filename,'.',extension],'r');  
while 1  
    tline = fgetl(fid_org);  
    if ~ischar(tline),   break,   end  
    fprintf(fid_new,'%s \n',tline);  
end  
fclose(fid_org); 
fclose(fid_new); 
 
%get move command depending on operating system
if(isunix)%if not a windows pc
    move='!mv ';
else
    move='!move ';
end

eval([move,dir,filename,'_header.',extension,' ',dir,filename,'.',extension]);

end%function  
  
