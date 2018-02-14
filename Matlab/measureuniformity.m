function [ uniformity ] = measureuniformity( vX )
maximum = max(vX);
minimum = min(vX);
uniformity = (1.0 - abs((maximum-minimum)/(maximum+minimum)));
end

