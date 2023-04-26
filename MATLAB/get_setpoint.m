function get_setpoint(topic,data)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

  assignin('base','payload',data);
  assignin('base','position',jsondecode(data));

end