%% Parameters Initialization - sm_3d_printer
%
% The following example derived from this Matlab® example:
% https://it.mathworks.com/help/physmod/sm/ug/cartesian_3d_printer.html
%
%% ============= Solid ============= %%
% Abbreviations:
% Center of Mass (CoM) 
% Moments of Inertia (MoI) 
% Product of Inertia (PoI)

% Initialize the Solid structure array by filling in null values.
smiData.Solid(14).mass = 0.0;
smiData.Solid(14).CoM = [0.0 0.0 0.0];
smiData.Solid(14).MoI = [0.0 0.0 0.0];
smiData.Solid(14).PoI = [0.0 0.0 0.0];
smiData.Solid(14).color = [0.0 0.0 0.0];
smiData.Solid(14).opacity = 0.0;
smiData.Solid(14).ID = '';

% Specify Guide_Rail_y
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(1).mass  = 0.423175000000000;  % kg
smiData.Solid(1).CoM   = [2.62787000000000e-11,4.59422000000000e-11,0.367779000000000];  % m
smiData.Solid(1).MoI   = [0.0193715000000000,0.0193715000000000,4.06246000000000e-05];  % kg*m^2
smiData.Solid(1).PoI   = [-2.06313000000000e-12,3.42327000000000e-06,5.56207000000000e-14];  % kg*m^2
smiData.Solid(1).color = [0.792156862745098 0.81960784313725488 0.93333333333333335];
smiData.Solid(1).opacity = 1;
smiData.Solid(1).ID = 'Guide_Rail_y*:*Default*:*Default';

% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(2).mass = 1.4072701459902404;  % kg
smiData.Solid(2).CoM = [0,0,0.368000000000000];  % m
smiData.Solid(2).MoI = [0.063605209695906,0.063605209695906,1.583178914239021e-04];  % kg*m^2
smiData.Solid(2).PoI = [0,0,0];  % kg*m^2
smiData.Solid(2).color = [0.898039215686275,0.917647058823529,0.929411764705882];
smiData.Solid(2).opacity = 1;
smiData.Solid(2).ID = 'Screw_Support*:*Default*:*Default';

% Specify Printing_head
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(3).mass = 0.137454491579698;  % kg
smiData.Solid(3).CoM = [2.414640000000000e-07,-0.032889500000000,0.014387900000000];  % m
smiData.Solid(3).MoI = [9.267760000000000e-05,5.639540000000000e-05,9.785860000000000e-05];  % kg*m^2
smiData.Solid(3).PoI = [-2.165530000000000e-05,4.338780000000000e-10,5.739020000000000e-10];  % kg*m^2
smiData.Solid(3).color = [0.50196078431372548 0.50196078431372548 0.50196078431372548];
smiData.Solid(3).opacity = 1;
smiData.Solid(3).ID = 'Printing_head*:*Default';

% Specify Sliding_Block
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(4).mass = 2.804350000000000;  % kg
smiData.Solid(4).CoM = [1.342570000000000e-08,0.010293300000000,0.002232070000000];  % m
smiData.Solid(4).MoI = [0.040236600000000,0.032875500000000,0.059026600000000];  % kg*m^2
smiData.Solid(4).PoI = [-0.001831760000000,4.120570000000000e-09,-1.601450000000000e-08];  % kg*m^2
smiData.Solid(4).color = [0 1 1];
smiData.Solid(4).opacity = 1;
smiData.Solid(4).ID = 'Sliding_Block*:*Default';

% Specify Screw_x
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(5).mass = 0.703698000000000;  % kg
smiData.Solid(5).CoM = [-3.700180000000000e-11,-5.765340000000000e-10,0.616274000000000];  % m
smiData.Solid(5).MoI = [0.090385600000000,0.090385600000000,6.730500000000000e-05];  % kg*m^2
smiData.Solid(5).PoI = [2.031370000000000e-10,5.951710000000000e-06,1.453210000000000e-13]; % kg*m^2
smiData.Solid(5).color = [0.69333333333333336 0.72784313725490202 0.80000000000000004];
smiData.Solid(5).opacity = 1;
smiData.Solid(5).ID = 'Screw_x*:*Default';

% Specify screw_z
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(6).mass = 0.468182000000000;  % kg
smiData.Solid(6).CoM = [5.537170000000000e-05,-3.691370000000000e-09,0.149947000000000];  % m
smiData.Solid(6).MoI = [0.003570280000000,0.003570280000000,4.587200000000000e-05];  % kg*m^2
smiData.Solid(6).PoI = [-7.024410000000000e-08,-7.854120000000000e-09,-1.191200000000000e-13];  % kg*m^2
smiData.Solid(6).color = [0.69333333333333336 0.72784313725490202 0.80000000000000004];
smiData.Solid(6).opacity = 1;
smiData.Solid(6).ID = 'screw_z*:*Default';

% Specify Printing_Bead
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(7).mass = 7.774750000000000;  % kg
smiData.Solid(7).CoM = [-3.571590000000000e-09,3.274610000000000e-09,5.843960000000000e-04];  % m
smiData.Solid(7).MoI = [0.288180000000000,0.243685000000000,0.524828000000000];  % kg*m^2
smiData.Solid(7).PoI = [1.287910000000000e-09,-1.404680000000000e-09,-2.459920000000000e-10];  % kg*m^2
smiData.Solid(7).color = [1 1 0];
smiData.Solid(7).opacity = 1;
smiData.Solid(7).ID = 'Printing_Bead*:*Default';

% Specify Guide_Rail_x
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(8).mass = 0.868074000000000;  % kg
smiData.Solid(8).CoM = [1.781120000000000e-19,-3.352690000000000e-20,0.618000000000000];  % m
smiData.Solid(8).MoI = [0.110561000000000,0.110561000000000,9.703280000000000e-05];  % kg*m^2
smiData.Solid(8).PoI = [-7.450580000000000e-21,1.210720000000000e-20,2.328310000000000e-22];  % kg*m^2
smiData.Solid(8).color = [0.8666666666666667 0.90980392156862744 1];
smiData.Solid(8).opacity = 1;
smiData.Solid(8).ID = 'Guide_Rail_x*:*Default';

% Specify Base
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(9).mass = 3.189742250720438;  % kg
smiData.Solid(9).CoM = [-1.029772096410384,0.210506415309284,0.211987838390462];  % m
smiData.Solid(9).MoI = [0.189421496406371,0.506624884048098,0.689135036773004];  % kg*m^2
smiData.Solid(9).PoI = [0,75.954896856531000,0];  % kg*m^2
smiData.Solid(9).color = [0.792156862745098 0.81960784313725488 0.93333333333333335];
smiData.Solid(9).opacity = 1;
smiData.Solid(9).ID = 'Base*:*Default';

% Specify Gantry
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(10).mass = 9.68041259778678;  % kg
smiData.Solid(10).CoM = [-1.115836130226037,0.210506415309283,0.541790869244187];  % m
smiData.Solid(10).MoI = [0.120522842473405,0.036950297667635,0.092857446100684];  % kg*m^2
smiData.Solid(10).PoI = [0,0.008122167275802,0];  % kg*m^2
smiData.Solid(10).color = [0.792156862745098 0.81960784313725488 0.93333333333333335];
smiData.Solid(10).opacity = 1;
smiData.Solid(10).ID = 'Gantry*:*Default';

% Specify Motor
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(11).mass = 0.556178000000000;  % kg
smiData.Solid(11).CoM = [-0.954432000000000,0.192950000000000,0.905997000000000];  % m
smiData.Solid(11).MoI = [7.402550000000000e-04,7.403800000000000e-04,5.078060000000000e-04];  % kg*m^2
smiData.Solid(11).PoI = [-1.545360000000000e-09,-1.656760000000000e-09,-4.677020000000000e-10];  % kg*m^2
smiData.Solid(11).color = [0.792156862745098 0.81960784313725488 0.93333333333333335];
smiData.Solid(11).opacity = 1;
smiData.Solid(11).ID = 'Motor*:*Default';

% Specify Screw_y
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(12).mass = 1.142570000000000;  % kg
smiData.Solid(12).CoM = [2.627870000000000e-11,4.594220000000000e-11,0.367779000000000];  % m
smiData.Solid(12).MoI = [0.052303000000000,0.052303000000000,1.096870000000000e-04];  % kg*m^2
smiData.Solid(12).PoI = [-5.570450000000000e-12,9.242830000000000e-06,1.501760000000000e-13];  % kg*m^2
smiData.Solid(12).color = [0.600000000000000,0.600000000000000,0.600000000000000];
smiData.Solid(12).opacity = 1;
smiData.Solid(12).ID = 'Screw_y*:*Default';

% Specify Guide_Rail_z
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(13).mass = 0.211717000000000;  % kg
smiData.Solid(13).CoM = [1.027480000000000e-19,0.150000000000000,-3.172760000000000e-19];  % m
smiData.Solid(13).MoI = [0.001599770000000,2.377990000000000e-05,0.001599770000000];  % kg*m^2
smiData.Solid(13).PoI = [0 0 0];  % kg*m^2
smiData.Solid(13).color = [0.8666666666666667 0.90980392156862744 1];
smiData.Solid(13).opacity = 1;
smiData.Solid(13).ID = 'Guide_Rail_z*:*Default';

% Specify Bracket
% Inertia Type - Custom
% Visual Properties - Simple
smiData.Solid(14).mass = 0.743558000000000;  % kg
smiData.Solid(14).CoM = [-3.150280000000000e-07,-0.038656000000000,-0.021517000000000];  % m
smiData.Solid(14).MoI = [0.003364330000000,0.003241630000000,0.004570550000000];  % kg*m^2
smiData.Solid(14).PoI = [8.771500000000000e-04,-1.796340000000000e-08,9.054830000000000e-09];  % kg*m^2
smiData.Solid(14).color = [0 1 1];
smiData.Solid(14).opacity = 1;
smiData.Solid(14).ID = 'Bracket*:*Default';

%% ============= Geometry Parameters ============= %%
% Initialize the geometry structure array by filling in null values
smiData.Geometry(3).value = 0.0;
smiData.Geometry(3).ID = '';

smiData.Geometry(1).value = 0.618*2; %m
smiData.Geometry(1).ID = 'X_base*:*Default';
smiData.Geometry(2).value = 0.6; %m
smiData.Geometry(2).ID = 'X_printing_bed*:*Default';
smiData.Geometry(3).value = 0.736; %m
smiData.Geometry(3).ID = 'Y_head_support*:*Default';
smiData.Geometry(4).value = 0.3; %m
smiData.Geometry(4).ID = 'Y_double_screw_table*:*Default';

%% ============= RigidTransform ============= %%
smiData.RF(1).Offset = -0.09; % m
smiData.RF(1).ID = 'connection between base and gantry*:*Default';

%Initialize the RigidTransform structure array by filling in null values.
smiData.offset(3).value = 0.0;
smiData.offset(3).ID = '';

smiData.offset(1).value = (smiData.Geometry(1).value-smiData.Geometry(2).value)/2; %m
smiData.offset(1).ID = 'x offset*:*Default';
smiData.offset(2).value = -(smiData.Geometry(3).value+smiData.Geometry(4).value)/2; %m
smiData.offset(2).ID = 'y offset*:*Default';
smiData.offset(3).value = -0.15;% m
smiData.offset(3).ID = 'z offset*:*Default';