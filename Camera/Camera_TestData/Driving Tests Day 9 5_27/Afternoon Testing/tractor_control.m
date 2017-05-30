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
        
% Loop through each data file, save relevant plots
for i =1:num_files;

M = dlmread(FILENAME{i}); % Load data file

[pathstr,name,ext] = fileparts(FILENAME{i}); % Obtain info from filename

PDF_NAME = name; %Name Output PDF Here

%PDF_NAME = 'Testing 5_17 Plots'; %Uncomment to make one pdf for multiple files

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
right_edge      = M(:,17);
theta_path      = M(:,18);
braking         = M(:,19);
nshift_theta_1  = M(:,20);
nshift_center_dist = M(:,21);

L = 2



x_cam = nshift_center_dist.*cos(nshift_theta_1)
y_cam = nshift_center_dist.*sin(nshift_theta_1)

x_fwheel = x_cam - L.*cos(theta_2)
y_fwheel = y_cam - L.*sin(theta_2)


% plot(x_cam,y_cam)
% hold on
% plot(x_fwheel,y_fwheel)

concentration = linspace(0,1, size(x_cam,1))




%% Plotting All Unique Paths
[C, ia, ic] = unique(a);
max_y_val = 0;
min_y_val = 0;
max_x_val = 0;


L = 2;

for i=1:length(ia)
    path_num = ia(i);
    start_point = nshift_center_dist(path_num)*cos(nshift_theta_1(path_num));
    start_point = start_point - L.*cos(theta_2(path_num))
    
    if (start_point >4)
    x = 0:.01:start_point;
    elseif (start_point >0)
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
    
    if (start_point > 4)
    p1 = plot(x+4,p);
    else
    p1 = plot(x,p);
    end
    
    hold on;
    p2 = plot(start_point,p(end),'r.','MarkerSize',20);
    xlabel('x (m)')
    ylabel('y (m)')
end
    p3 = plot(0,p(1),'g.','MarkerSize',20);
    h = zeros(2, 1);
    h(1) = plot(0,0,'or', 'visible', 'off');
    h(2) = plot(0,0,'og', 'visible', 'off');
    legend(h, 'Start Point','End Point');
    grid on
    %export_fig(PDF_NAME,'-transparent','-pdf','-append'



scatter(x_cam,y_cam, size(x_cam,1), concentration);
hold on
scatter(x_fwheel,y_fwheel,'d')
set(gca,'CLim',[0 1]);
    axis([0 14 -6 6])


% plot(theta_2)
% hold on
end



