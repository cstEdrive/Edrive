close all
clear all
clc

load input_data.txt
load output_data.txt

N = length(output_data);

c_matlab = fft(input_data,N);


c = zeros(N,1);
j = sqrt(-1);    
c = output_data(:,2)+output_data(:,3)*j;



if (norm(c-c_matlab,2) > 1e-10)
    disp('Trying fftshift')
    c_matlab = fftshift(fft(input_data,N));
end

norm(c-c_matlab,2)

figure
plot(input_data)

figure
stem(abs(c))
hold on
stem(abs(c_matlab),'r')