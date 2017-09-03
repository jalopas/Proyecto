% Canal de Contaminación Atmosférica
readChId = 101717;   % ID del canal
readAPIKey = '5TYGJ7MFYUO5D5I2';
% Canal Matlab para Contaminación Atmosférica
writeChId = 316681;  % ID del canal
writeAPIKey = '0PGGKZU0NR8NME7X'; 

% Se leen los datos de la última lecturas
Temp_C = thingSpeakRead(readChId,'Fields',1,'ReadKey', readAPIKey);
P_mBa  = thingSpeakRead(readChId,'Fields',2,'ReadKey', readAPIKey);
NO2    = thingSpeakRead(readChId,'Fields',6,'ReadKey', readAPIKey);
CO     = thingSpeakRead(readChId,'Fields',7,'ReadKey', readAPIKey);
NH4    = thingSpeakRead(readChId,'Fields',8,'ReadKey', readAPIKey);

% Pesos atómicos de los elementos.
PA_H=1;
PA_C=12;
PA_N=14;
PA_O=16;

% Calcular el Peso Molecular de los gases.
PM_NO2 = PA_N + 2 * PA_O;
PM_CO  = PA_C + PA_O;
PM_NH4 = PA_N + 4 * PA_H;

% Calcular el Volumen Molar de los gases. Se usa la fórmula P*V=N*R*T => V=(N*R*T)/P
N = 1; % Numero de Moles
R = 62.32; % Constante
T_K      = Temp_C + 273.15;  % Temperatura en Kelvin
MmHg_MBa = 0.75006375541921;
P_mmHg   = P_mBa * MmHg_MBa; % Presion en mmHg.

VM_NO2 = (N * R * T_K) / P_mmHg;
VM_CO  = (N * R * T_K) / P_mmHg;
VM_NH4 = (N * R * T_K) / P_mmHg;

% Calcular los mg/m3 del gas. Se usa la formula 
% mg/m3 =(Peso Molecular/Volumen Molar)* p.p.m.
NO2_mg_m3 = (PM_NO2/VM_NO2)*NO2;
CO_mg_m3  = (PM_CO/VM_CO)*CO;
NH4_mg_m3 = (PM_NH4/VM_NH4)*NH4;

display(NO2_mg_m3, 'NO2');
display(CO_mg_m3, 'CO');
display(NH4_mg_m3, 'NH4');

% Muestra los resultados en una gráfica en el canal Matlab para contaminación atmosférica.
thingSpeakWrite(writeChId,[NO2_mg_m3,CO_mg_m3,NH4_mg_m3],'Fields',[1,2,3],'Writekey', writeAPIKey);






%thingSpeakPlot(timeStamps,dewPointData,'xlabel','TimeStamps',...
%    'ylabel','Measured Values','title','Dew Point Measurement',...
%    'Legend',{'Temperature','Humidity','Dew Point'},'grid','on');



https://es.mathworks.com/help/thingspeak/analyze-your-data.html
http://www.emagister.com/uploads_user_home/Comunidad_Emagister_4761_quimica.pdf


https://thingspeak.com/apps/matlab_analyses/58563/edithttps://thingspeak.com/apps/matlab_analyses/templates

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