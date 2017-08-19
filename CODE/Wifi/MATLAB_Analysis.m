
   % CÓDIGO FUENTE ANALISIS DE MATLAB PARA CONTAMINACIÓN ATOMSFÉRICA

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

% Calcular el Volumen Molar de los gases. 
% Se usa la fórmula P*V=N*R*T => V=(N*R*T)/P
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

% Muestra los resultados en una gráfica en el canal Matlab
% para contaminación atmosférica.
thingSpeakWrite(writeChId,[NO2_mg_m3,CO_mg_m3,NH4_mg_m3],'Fields',[1,2,3],'Writekey', writeAPIKey);
