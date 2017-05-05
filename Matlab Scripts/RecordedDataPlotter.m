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

PDF_NAME ='AngledLeft Filtered L1vsL2'; %

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
        
% Loop through each data file, save relevatnt plots
for i =1:num_files;

M = dlmread(FILENAME{i}); 

% Obtain info from filename
[pathstr,name,ext] = fileparts(FILENAME{i});

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
possible_path   = M(:,11);
dis_LID         = M(:,12);
t1_LID          = M(:,13);
t2_LID          = M(:,14);
kp_flag         = M(:,15);

if (isempty(M(1,11)) == 0)
    path_possible = M(:,11);
end

nan_vals = isnan(a);
idx = find(nan_vals == 0);
a = a(idx);

nan_vals = isnan(b);
idx = find(nan_vals == 0);
b = b(idx);

% Plotting Initial Path
[C, ia, ic] = unique(a);
figure
x = 0:1:10;
p = a(1).*x.^2+b(1).*x.^3;
plot(x,p)
hold on;
plot(10,p(11),'r.','MarkerSize',20)
hold on;
plot(0,p(1),'g.','MarkerSize',20)
xlabel('X (m)')
ylabel('y (m)')
title(strcat(name,' Initial Path'),'Interpreter', 'none')
grid on
export_fig(PDF_NAME,'-pdf','-transparent','-append')

% Plotting camera L1 & L2
figure
plot(L1)
hold on
plot(L2)
title(strcat(name,' Camera L1 vs L2'),'Interpreter', 'none')
legend('L1','L2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% Plotting average L1 & L2
figure
plot(left_mean)
hold on
plot(right_mean)
title(strcat(name,' Camera L1 vs L2 rolling average'),'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m)')
legend('L1','L2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% Plotting camera theta_1 & theta_2
figure
plot(theta_1)
hold on
plot(theta_2)
xlabel('index')
ylabel('Distance (m)')
title(strcat(name,'   Camera Theta_1 vs Theta_2'),'Interpreter', 'none')
legend('\theta 1','\theta 2')
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% % Plotting Lidar theta_1 & theta_2
% figure
% plot(t1_LID)
% hold on
% plot(t2_LID)
% xlabel('Index')
% ylabel('Distance (m)')
% title(strcat(name,'   LIDAR Theta_1 vs Theta_2'),'Interpreter', 'none')
% legend('\theta 1','\theta 2')
% export_fig(NAME,'-transparent','-pdf','-append')

% % Plotting camera vs LIDAR theta_1
% figure
% plot(theta_1)
% hold on
% plot(t1_LID)
% xlabel('index')
% ylabel('Distance (m)')
% title(strcat(name,'   Camera vs LIDAR Theta_1'),'Interpreter', 'none')
% legend('Camera','LIDAR')
% export_fig(NAME,'-transparent','-pdf','-append')

% % Plotting camera vs LIDAR theta_2
% figure
% plot(theta_2)
% hold on
% plot(t2_LID)
% xlabel('index')
% ylabel('Distance (m)')
% title(strcat(name,'   Camera vs LIDAR Theta_2'),'Interpreter', 'none')
% legend('Camera','LIDAR')
% export_fig(NAME,'-transparent','-pdf','-append')

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

end

close all

%% Testing Section
% plot(x,p)
% hold on
% plot(theta_2)
% hold on
% plot(steer)
% grid

    L = 2;
    SPEED = 500;
    delay = 110;
    RMIN = 8;
    
    dist_grad = ((SPEED /3600)*(1000/delay));

    x_cam         = center_dist .* cos(theta_1);
    y_cam         = center_dist .* sin(theta_1);
    
    x_fwheel      = x_cam - L.*cos(theta_2);
    y_fwheel      = y_cam - L.*sin(theta_2);

    y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
    y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
    
    xdis = sqrt(L^2 - (y_cam_path - y_fwheel_path).^2);
    
    theta_path = atan((y_cam_path - y_fwheel_path)./xdis); 
    
    angle_diff    = (theta_path - theta_2);
    
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
    plot(angle_diff)
    hold on
    plot(steer)
    hold on
    plot(new_steering./8192)
    legend('Recorded \theta 2','\theta P','\thetaP - \theta2','Recorded Steering Command','Updated Steering Command')
    export_fig('Steering','-transparent','-pdf','-append')
    
    
%   plot(y_fwheel)
%   hold on
%   plot(y_cam)
    
%     figure
%     plot(angle_diff*10)
%     hold on
%     plot(theta_2*10)
%     hold on
%     plot(steer)
%     grid
%     legend('Angle Diff','\theta 2')
    


