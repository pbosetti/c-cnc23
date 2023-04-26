%% Single axis example 
clc; clear all; close all;

%% Model parameters
% Base geometry
lb = 500; % [mm]
wb = 25;
tb = 15; 
% Cursor geometry
t = tb;
w = wb;
l = w;
% Lead screw parameter
ls_pitch = 3.0; % [mm/rev]

%% DC motor paramters
w_m = [0, 1000, 2000, 3000]; % [rpm]
t_m = [1, 1, 0.95, 0] * 200; % [Nm]

figure
plot(w_m, t_m)
xlabel("[rpm]")
ylabel("[Nm]")
grid on
title("DC motor torque curve")
