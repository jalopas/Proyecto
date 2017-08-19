% https://es.mathworks.com/help/thingspeak/analyze-your-data.html
readChId_1 = 101717
readKey_1  = '5TYGJ7MFYUO5D5I2'
readChId_2 = 316681
readKey_2  = 'W9GNN5DAC8AZ5B7W'

[NO2_Data,timeStamps] = thingSpeakRead(readChId_1,'fields',[1,6],...
    'NumPoints',100,'ReadKey',readKey_1);

%[NO2_Data_Matlab,timeStamps] = thingSpeakRead(readChId_2,'fields',1,...
%    'NumPoints',10,'ReadKey',readKey_2);
    
thingSpeakPlot(timeStamps,NO2_Data,'xlabel','Time',...
    'ylabel','Valores','title','Medidas de NO2',...
    'Legend',{'Temperatura','NO2'},'grid','on');    