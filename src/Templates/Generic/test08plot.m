load test08.txt
figure
plot(test08(:,1),test08(:,2),'r')
hold on
plot(test08(:,1),test08(:,3),'b')

figure
plot(test08(:,1),-test08(:,2),'r')
hold on
plot(test08(:,1),test08(:,4),'b')
