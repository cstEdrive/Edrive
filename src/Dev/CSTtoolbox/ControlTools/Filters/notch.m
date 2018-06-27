function Cn=notch(f_num,f_den,beta_num,beta_den);
%lowpass filter
% Clp=notch(fnum,fden,betanum,betaden);
s=tf('s');
w_num=f_num*2*pi;
w_den=f_den*2*pi;
Cn=(s^2/w_num^2 + 2*beta_num*s/w_num +1)/(s^2/w_den^2 + 2*beta_den*s/w_den +1);

end %function