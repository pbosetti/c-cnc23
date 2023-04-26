%% Cartesian 3D Printer - Digital twin
%
% The following example derived from this Matlab® example:
% https://it.mathworks.com/help/physmod/sm/ug/cartesian_3d_printer.html
%
% This example aims to show a basic digital twin between a real cartesian
% 3D printer and a simulated one. The real printer behaviour is represented 
% by the c-cnc while the simulated one is represented by a Matlab/Simulink
% simulation.
%
%% Project setup
clc; clear all;

% Add printer data folders
addpath('Printer/Images')
addpath('Printer/CAD')
addpath('s-functions')

%% MQTT client
M = mqttclient('tcp://localhost'); % Broker connection

% Initialize subscribers
sub = subscribe(M, 'c-cnc/setpoint', Callback=@get_setpoint); % Subscribe to set-point topic

% Publish first set point
write(M, 'c-cnc/setpoint', '{"x":0.000,"y":0.000,"z":-100.000,"rapid":false}'); % Set-point init
% MQTT JSON message description
% 1. c-cnc is the root
% 2. setpoint is the topic
% 3. {x,y,z,rapid} is the message formatted as x, y and z (set point) and
%    a rapid movement flag (false=no rapid movement, true=rapid movement).

%% Test message
% Publish dummy set-point messages
% write(M, 'c-cnc/setpoint', '{"x":200,"y":200,"z":200.000,"rapid":false}'); 

%% Set printer parameters
sm_3d_printer_parameters;

%% Motor parameters
w_m = [0, 1000, 2000, 3000]; % Rotational speed [rpm]
t_m = [1, 1, 0.95, 0] * 200; % Torque [Nm]

figure
plot(w_m, t_m)
xlabel("[rpm]")
ylabel("[Nm]")
grid on
title("DC motor torque curve")

%% Open Simulink model and Mechanics explorer

open_system('sm_3d_printer');
set_param('sm_3d_printer','SimMechanicsOpenEditorOnUpdate','on');