function test09stat(varargin)

nmissed = 0;
if nargin == 0
    fs = 4000;
else 
    fs = varargin{1};
end

load sampletimes
for i = 2 : length(sampletimes)
    diff(i-1,1) = sampletimes(i) - sampletimes(i-1);
end
figure
plot(diff)
hold on
plot(ones(length(sampletimes),1)./fs,'r')
plot(2.*ones(length(sampletimes),1)./fs,'g')
for i = 1 : length(diff)
    if( (diff(i) > 2/fs) )
        plot(i,diff(i),'ro')
        nmissed=nmissed+1;
    end    
end
        

mean_sample_freq = 1 / mean(diff)
nmissed
sum(diff > 2/fs)
