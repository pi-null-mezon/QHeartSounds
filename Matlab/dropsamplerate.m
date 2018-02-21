function [ outputvector, windowsize ] = dropsamplerate( inputvector, samplerate, targetsamplerate)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

windowsize = samplerate / targetsamplerate;

tmpvector = zeros(size(inputvector,1),1);

tmpsum = 0.0;
for i=1:windowsize
    tmpsum = tmpsum + inputvector(i);
end
tmpvector(1) = tmpsum / windowsize;

for i = 2:(size(inputvector,1)-windowsize)
    tmpsum = tmpsum - inputvector(i-1) + inputvector(i+windowsize);
    tmpvector(i) = tmpsum / windowsize; 
end

outputvector = tmpvector(1:windowsize:end);

end

