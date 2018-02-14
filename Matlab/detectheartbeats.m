function [ is_heart_beating_sound ] = detectheartbeats( filename )

Data = csvread(filename,1,0);
load('lpfe125','ellipticlpf');
S = abs(diff(filter(ellipticlpf,Data(:,2))));
Stdev = std(S);
S = S / Stdev;

Bin = zeros(size(S));
timethresh = 0.275;
timemark = 0;
vI = 0;
intervals = 1;

for i = 1:size(Bin,1)
   if (S(i) > 2.5)
       if (Data(i,1) - timemark) > timethresh 
            vI(intervals) = Data(i,1) - timemark;
            intervals = intervals + 1;
            Bin(i) = 10;
            timemark = Data(i,1);
       end
   end
end
if(Data(size(Data,1),1) - timemark) > timethresh/2.0
    vI(intervals) = Data(size(Data,1),1) - timemark;
end

figure('Name',filename)
hold on
plot(S);
plot(Bin);

uniformity = measureuniformity(vI);
display('--------------');
display(['File: ' filename]);
hr = 0.0;
is_heart_beating_sound = false;
if uniformity > 0.333  
    hr = 60.0/(2.0*mean(vI)); 
    if hr > 55 && hr < 135
        is_heart_beating_sound = true;        
    end
end

display(['Pulse rate: ' num2str(round(hr)) ' bpm']);
display(['Uniformity of pulses: ' num2str(uniformity)]);

if is_heart_beating_sound
    title(['HR: ' num2str(round(hr)) ' bpm; UN:' num2str(uniformity*100.0) ' %']);
else
    title(['HEART BEATS NOT DETECTED; ','UN:' num2str(uniformity*100.0) ' %']);
end

end

