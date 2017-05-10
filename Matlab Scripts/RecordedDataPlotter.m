%% Script to read multiple data files and plot relevant information

% Note: In order to use this script you must download the export_fig
% function found here: https://www.mathworks.com/matlabcentral/fileexchange/23629-export-fig
% and add it to your Matlab path

% Instructions
% To use this script first navigate your current folder to the
% location  of the files you wish to process, then
% change the name field below for the name of the ouput PDF.
% Run the script and select all the files you wish to process.

clc
clearvars

[FILENAME, PATHNAME, FILTERINDEX] = uigetfile( ...
        {'*.txt', 'Text (*.txt)'; ...
        '*.xls', 'Excel (*.xls)'; ...
        '*.*', 'All Files (*.*)'}, ...
        'Please select all the files you wish to generate a report from', ...
        'MultiSelect', 'on');
    
    if (ischar(FILENAME) == 1)
        FILENAME = cellstr(FILENAME);
    end
    

    num_files = length(FILENAME);
        
%% Loop through each data file, save relevant plots
for i =1:num_files;

M = dlmread(FILENAME{i}); % Load data file

[pathstr,name,ext] = fileparts(FILENAME{i}); % Obtain info from filename

PDF_NAME =name; %Name Output PDF Here

% Parsing from matrix into vectors
L1              = M(:,1);
left_mean       = M(:,2);
L2              = M(:,3);
right_mean      = M(:,4);
center_dist     = M(:,5);
theta_1         = M(:,6);
theta_2         = M(:,7);
a               = M(:,8);
b               = M(:,9);
steer           = M(:,10);

if (isempty(M(1,11)) == 0)
    path_possible = M(:,11);
end

if (isempty(M(1,12)) == 0)
    dis_LID         = M(:,12);
end


t1_LID          = M(:,13);
t2_LID          = M(:,14);
kp_flag         = M(:,15);

left_edge       = M(:,16);
right_edge       = M(:,17);
theta_path       = M(:,18);


if (isempty(M(1,11)) == 0)
    path_possible = M(:,11);
end

nan_vals = isnan(a);
idx = find(nan_vals == 0);
a = a(idx);

nan_vals = isnan(b);
idx = find(nan_vals == 0);
b = b(idx);

%%






%% Plotting Initial Path
[C, ia, ic] = unique(a);


for i=1:length(ia)
    path_num = ia(i);

    start_point = center_dist(path_num)*cos(theta_1(path_num));

    x = 0:.01:start_point;
    p = a(path_num).*x.^2+b(path_num).*x.^3;
    plot(x,p)
    hold on;
    plot(start_point,p(end),'r.','MarkerSize',20)
    hold on;
    xlabel('x (m)')
    ylabel('y (m)')
end
    plot(0,p(1),'g.','MarkerSize',20)
    
    title(strcat(name,' Initial Path'),'Interpreter', 'none')
    grid on
    
    %%
export_fig(PDF_NAME,'-pdf','-transparent','-append')

%% Determing number of paths calculated
index = find(center_dist > 2);
new_a = a(1:100);
Number_Of_Unique_Paths = length(unique(new_a))




%% Counting King Pin Flag
if (isempty(find(kp_flag, 1)))
    King_Pin_Detected = false;
else
    King_Pin_Detected = true;
end
T = table(Number_Of_Unique_Paths, King_Pin_Detected);
figure
% Get the table in string form.
TString = evalc('disp(T)');
% Use TeX Markup for bold formatting and underscores.
TString = strrep(TString,'<strong>','\bf');
TString = strrep(TString,'</strong>','\rm');
TString = strrep(TString,'_','\_');
% Get a fixed-width font.
FixedWidth = get(0,'FixedWidthFontName');
% Output the table using the annotation command.
annotation(gcf,'Textbox','String',TString,'Interpreter','Tex',...
    'FontName',FixedWidth,'Units','Normalized','Position',[0 0 .5 .2]);
export_fig(PDF_NAME,'-pdf','-transparent','-append')

%% Plotting left_edge vs right_edge
figure
plot(left_edge)
hold on
plot(right_edge)
hold on
plot(left_mean*100)
hold on
plot(right_mean*100)
title(strcat(name,' Left_Edge vs Right_Edge'),'Interpreter', 'none')
legend({'Left_Edge','Right_Edge','L1 x 100', 'L2 x 100'}, 'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m) & Pixel')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting Lidar Distance
figure
plot(dis_LID)
title(strcat(name,' LIDAR Distance'),'Interpreter', 'none')
legend('Upper LIDAR')
xlabel('Index')
ylabel('Distance (m)')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting Center Dist
figure
plot(center_dist)
title(strcat(name,' Center Distance'),'Interpreter', 'none')
legend('Center Dist')
xlabel('Index')
ylabel('Distance (m)')
export_fig(PDF_NAME,'-transparent','-pdf','-append')


%% Plotting Average L1 & L2
figure
plot(left_mean)
hold on
plot(right_mean)
title(strcat(name,' Camera L1 vs L2 rolling average'),'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m)')
legend('L1','L2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting camera theta_1 & theta_2
figure
plot(theta_1)
hold on
plot(theta_2)
xlabel('index')
ylabel('Distance (m)')
title(strcat(name,'   Camera Theta_1 vs Theta_2'),'Interpreter', 'none')
legend('\theta 1','\theta 2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting Lidar theta_1 & theta_2
figure
plot(t1_LID)
hold on
plot(t2_LID)
xlabel('Index')
ylabel('Distance (m)')
title(strcat(name,'   LIDAR Theta_1 vs Theta_2'),'Interpreter', 'none')
legend('\theta 1','\theta 2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting camera vs LIDAR theta_1
figure
plot(theta_1)
hold on
plot(t1_LID)
xlabel('index')
ylabel('Distance (m)')
title(strcat(name,'   Camera vs LIDAR Theta_1'),'Interpreter', 'none')
legend('Camera','LIDAR')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting camera vs LIDAR theta_2
figure
plot(theta_2)
hold on
plot(t2_LID)
xlabel('index')
ylabel('Distance (m)')
title(strcat(name,'   Camera vs LIDAR Theta_2'),'Interpreter', 'none')
legend('Camera','LIDAR')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% Plotting camera location
figure
x1 = center_dist.*cos(theta_1);
y1 = center_dist.*sin(theta_1);
plot(x1,y1)
title(strcat(name,' Camera Position'),'Interpreter', 'none')
ylabel('Distance (m)')
xlabel('Distance (m)')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% Plotting steering command
steer = steer./8192; %Degrees
figure
plot(steer)
title(strcat(name,' Steering Command'),'Interpreter', 'none')
xlabel('Index')
ylabel('Turns (Clockwise Positive) 1 = 360^o turn')
export_fig(PDF_NAME,'-transparent','-pdf','-append')


%% Plotting calculated steering value
    L = 2;
    SPEED = 500;
    delay = 110;
    RMIN = 7.2;
    
    new_theta_1 = theta_1;
    
    dist_grad = ((SPEED /3600)*(1000/delay));

    x_cam         = center_dist .* abs(cos(new_theta_1));
    y_cam         = center_dist .* sin(new_theta_1);
    
    x_fwheel      = x_cam - L.*cos(theta_2);
    y_fwheel      = y_cam - L.*sin(theta_2);

    y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
    y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
    
    xdis = sqrt(L^2 - (y_cam_path - y_fwheel_path).^2);
    
    theta_path_calc = atan((y_cam_path - y_fwheel_path)./dist_grad); 
    
    angle_diff    = (theta_path_calc - theta_2);
    
    chan_f = (RMIN/dist_grad)*angle_diff;
    
    for i =1:length(chan_f)
    if(chan_f(i) > 1)
	chan_f(i) = 1;
    end
    if(chan_f(i) < -1)
	chan_f(i) = -1;
    end
    end
    
	new_steering = 24000*(chan_f);

    plot(theta_2)
    hold on
    plot(theta_path)
    hold on
    plot(theta_path_calc)
    hold on
    plot(steer)
    hold on
    plot(new_steering./8192)
    hold on
    plot(path_possible,':mo')
    legend('Recorded \theta 2','Recorded \theta P','Calculated \theta P','Recorded Steering Command','Calculated Steering Command','Path Flag')
    title(strcat(name,' Calculated vs. Actual Steering'),'Interpreter', 'none')
    xlabel('Index')
    export_fig(PDF_NAME,'-transparent','-pdf','-append')
    
    end

close all


%% theta_1 recalculation
T1 = -0.1565;
idx = 1;
d = 9.8133;
ax = 4;

d_prime = sqrt(d^2 + ax^2 -2*d*ax*cos(T1))

tB = acos((ax^2 + d_prime^2 - d^2)/(2*ax*d_prime))

new_t1 = pi - tB









