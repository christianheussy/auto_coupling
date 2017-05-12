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

PDF_NAME = '5_9_Fixed_Path_Results'; %Name Output PDF Here

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

if (~isempty(M(1,12)))
    dis_LID         = M(:,12);
end


t1_LID          = M(:,13);
t2_LID          = M(:,14);
kp_flag         = M(:,15);

left_edge       = M(:,16);
right_edge       = M(:,17);
theta_path       = M(:,18);


nan_vals = isnan(a);
idx = find(nan_vals == 0);
a = a(idx);

nan_vals = isnan(b);
idx = find(nan_vals == 0);
b = b(idx);

%%






%% Plotting All Unique Paths
[C, ia, ic] = unique(a);
max_y_val = 0;
min_y_val = 0;
max_x_val = 0;

for i=1:length(ia)
    path_num = ia(i);
    start_point = center_dist(path_num)*cos(theta_1(path_num));
    if (start_point >0)
    x = 0:.01:start_point;
    end
    if (start_point <0)
    x = 0:-.01:start_point;
    end
    
    p = a(path_num).*x.^2+b(path_num).*x.^3;
    if (max(p) > max_y_val)
    max_y_val =  max(p);
    end
    if (min(p) < min_y_val)
        min_y_val = min(p);
    end
    if (start_point > max_x_val)
        max_x_val = start_point;
    end
    
    plot(x,p)
    hold on;
    plot(start_point,p(end),'r.','MarkerSize',20)
    hold on;
    xlabel('x (m)')
    ylabel('y (m)')
end
    plot(0,p(1),'g.','MarkerSize',20)
    axis([0 max_x_val min_y_val max_y_val])
    title(strcat(name,' All Paths'),'Interpreter', 'none')
    h = zeros(2, 1);
    h(1) = plot(0,0,'or', 'visible', 'off');
    h(2) = plot(0,0,'og', 'visible', 'off');
    legend(h, 'Start Point','Trailer');
    grid on
    export_fig(PDF_NAME,'-pdf','-transparent','-append')

%% Determing number of paths calculated
index = find(center_dist > 2);
new_a = a(index);
Number_Of_Unique_Paths = length(unique(new_a));

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
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting Lidar vs Camerea Distance
figure
plot(dis_LID)
hold on
plot(center_dist)
title(strcat(name,' LIDAR Distance vs Camera Center Distance'),'Interpreter', 'none')
legend({'Upper LIDAR Distance','Center_Dist'},'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m)')
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% %% Plotting Center Dist
% figure
% title(strcat(name,' Center Distance'),'Interpreter', 'none')
% legend('Center Dist')
% xlabel('Index')
% ylabel('Distance (m)')
% export_fig(PDF_NAME,'-transparent','-pdf','-append')


%% Plotting Average L1 & L2
figure
plot(left_mean)
hold on
plot(right_mean)
title(strcat(name,' Camera L1 vs L2'),'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m)')
legend('L1','L2')
grid on
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
grid on
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
grid on
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
grid on
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
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')

% % Plotting camera location
% figure
% x1 = center_dist.*cos(theta_1);
% y1 = center_dist.*sin(theta_1);
% plot(x1,y1)
% title(strcat(name,' Camera Position'),'Interpreter', 'none')
% ylabel('Distance (m)')
% xlabel('Distance (m)')
% export_fig(PDF_NAME,'-transparent','-pdf','-append')

% % Plotting steering command
% steer = steer./8192; %Degrees
% figure
% plot(steer)
% title(strcat(name,' Steering Command'),'Interpreter', 'none')
% xlabel('Index')
% ylabel('Turns (Clockwise Positive) 1 = 360^o turn')
% export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting theta_1/2 l1/2 & theta_path
figure
plot(theta_2)
hold on
plot(theta_1)
hold on
plot(theta_path)
hold on
plot(left_mean/10)
hold on
plot(right_mean/10)
title(strcat(name,'   Camera Angles and Edge Distances'),'Interpreter', 'none')
legend('Rec \theta 2', 'Rec \theta 1', 'Rec \theta P', 'L1/10', 'L2/10')
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting calculated steering value
    L = 2;
    SPEED = 500;
    delay = 110;
    RMIN = 7.2;
    
    
    dist_grad = ((SPEED /3600)*(1000/delay));

    x_cam         = center_dist .* abs(cos(theta_1));
    y_cam         = center_dist .* sin(theta_1);
    
    x_fwheel      = x_cam - L.*cos(theta_2);
    y_fwheel      = y_cam - L.*sin(theta_2);

    y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
    y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
    
    xdis = sqrt(L^2 - (y_cam_path - y_fwheel_path).^2);
    
    theta_path_calc = atan((y_cam_path - y_fwheel_path)./xdis); 
    
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

    figure
    plot(theta_2)
    hold on
    plot(theta_path)
    hold on
    plot(theta_path_calc)
    hold on
    plot(steer./8192)
    hold on
    plot(new_steering./8192)
    hold on
    plot(path_possible,':')
    legend('Recorded \theta 2','Recorded \theta P','Calculated \theta P','Recorded Steering Command',...
        'Calculated Steering Command','Path Flag (1 = true)')
    title(strcat(name,' Calculated vs. Actual Steering'),'Interpreter', 'none')
    xlabel('Index')
    grid on
    export_fig(PDF_NAME,'-transparent','-pdf','-append')
    hold off
    
    end

close all


% % theta_1 recalculation
% T1 = -0.1565;
% idx = 1;
% d = 9.8133;
% ax = 4;
% d_prime = sqrt(d^2 + ax^2 -2*d*ax*cos(T1))
% tB = acos((ax^2 + d_prime^2 - d^2)/(2*ax*d_prime))
% new_t1 = pi - tB



