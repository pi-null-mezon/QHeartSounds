function [ is_heart_beating_sound ] = detectheartbeats( filename )

Data = csvread(filename,1,0);
[Y, W] = dropsamplerate(Data(:,2),8000,200);
S = abs(diff(Y));
%figure
%plot(Y);
S = S / std(S);

Bin = zeros(size(S));
timethresh = 0.175;
timemark = 0;
vI = 0;
intervals = 1;
T = Data(1:W:end,1); 

for i = 1:size(Bin,1)
   if(S(i) > 2.0)
       timeshift = T(i,1) - timemark; 
       if timeshift > timethresh 
            vI(intervals) = timeshift;
            intervals = intervals + 1;
            Bin(i) = 10;
            timemark = T(i,1);
       end
   end
end
if(T(end) - timemark) > timethresh/2.0
    vI(intervals) = T(end) - timemark;
end

figure('Name',filename)
hold on
plot(S);
plot(Bin);
hold off

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

