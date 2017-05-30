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



%% Plotting All Unique Paths
[C, ia, ic] = unique(a);
max_y_val = 0;
min_y_val = 0;
max_x_val = 0;


L = 2;

for i=1:length(ia)
    path_num = ia(i);
    start_point = center_dist(path_num)*cos(theta_1(path_num));
    start_point = start_point - L.*cos(theta_2(path_num))
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
    
    p1 = plot(x,p);
    hold on;
    p2 = plot(start_point,p(end),'r.','MarkerSize',20);
    xlabel('x (m)')
    ylabel('y (m)')
end
    p3 = plot(0,p(1),'g.','MarkerSize',20);
    axis([0 max_x_val min_y_val max_y_val])
    h = zeros(2, 1);
    h(1) = plot(0,0,'or', 'visible', 'off');
    h(2) = plot(0,0,'og', 'visible', 'off');
    legend(h, 'Start Point','End Point');
    grid on
    export_fig(PDF_NAME,'-transparent','-pdf','-append')
   hold off
    
%     axis([0 4 0 2])
%     
% 
%     C = linspecer(3);
%     
%     set(p1 , ...
%     'Color'           , C(1,:),...
%     'LineWidth'       , 2           );
% 
%     set(p2                            , ...
%   'LineStyle'       , 'none'      , ...
%   'MarkerSize'      , 35           , ...
%   'Marker'          , '.'         , ...
%   'Color'           , C(2,:)  );
% 
% 
%     set(p3                            , ...
%   'LineStyle'       , 'none'      , ...
%   'MarkerSize'      , 35           , ...
%   'Marker'          , '.'         , ...
%   'Color'           , C(3,:) );
% 
% 
%     set(gca,...
%     'Units','normalized',...
%     'YTick',-3:.5:3,...
%     'Position',[.15 .2 .75 .7],...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Helvetica');
% 
%     ylab = ylabel({'Y (m)'},...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     xlab = xlabel('X (m)',...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     leg = legend(...
%         [p1, p2, p3],...
%         'Path','Start Point','End Point')
%     
%     set([xlab, ylab], ...
%         'FontName'   , 'Times');
%     
%     set(gca, ...
%   'Box'         , 'off'     , ...
%   'TickDir'     , 'out'     , ...
%   'TickLength'  , [.02 .02] , ...
%   'XMinorTick'  , 'off'      , ...
%   'YMinorTick'  , 'off'      , ...
%   'YGrid'       , 'on'      , ...
%   'XColor'      , [.3 .3 .3], ...
%   'YColor'      , [.3 .3 .3], ...
%   'YTick'       , 0:.5:2, ...
%   'LineWidth'   , 1         );
%     
% export_fig('path_plot_new','-transparent','-png','-append')
    
    

%% Determing number of paths calculated


Number_Of_Unique_Paths = length(unique(a));

% %% Counting King Pin Flag
% if (isempty(find(kp_flag, 1)))
%     King_Pin_Detected = false;
% else
%     King_Pin_Detected = true;
% end
% 
% figure
% T = table(Number_Of_Unique_Paths, King_Pin_Detected);
% % Get the table in string form.
% TString = evalc('disp(T)');
% % Use TeX Markup for bold formatting and underscores.
% TString = strrep(TString,'<strong>','\bf');
% TString = strrep(TString,'</strong>','\rm');
% TString = strrep(TString,'_','\_');
% % Get a fixed-width font.
% FixedWidth = get(0,'FixedWidthFontName');
% % Output the table using the annotation command.
% annotation(gcf,'Textbox','String',TString,'Interpreter','Tex',...
%     'FontName',FixedWidth,'Units','Normalized','Position',[0 0 .5 .2]);
% export_fig(PDF_NAME,'-pdf','-transparent','-append')
% hold off

%% Plotting left_edge vs right_edge
figure
plot(left_edge)
hold on
plot(right_edge)
plot(left_mean*100)
plot(right_mean*100)
title(strcat(name,' Left_Edge vs Right_Edge'),'Interpreter', 'none')
legend({'Left_Edge','Right_Edge','L1 x 100', 'L2 x 100'}, 'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m) & Pixel')
grid on
export_fig(PDF_NAME,'-transparent','-png','-append')
hold off

%% Plotting Lidar vs Camerea Distance


   


%%


% for i = 1:length(dis_LID)
% 
% 
% outlier_idx =  abs(y - median(y)) > 1*std(y) % Find outlier idx
%  % Linearly interpolate over outlier idx for x
% y(outlier_idx) = interp1(all_idx(~outlier_idx), y(~outlier_idx), all_idx(outlier_idx)) % Do the same thing for y
% end
% 
% plot(x,y)
% hold on

%%
% for i =1:80
%     aNum = mean(dis_LID(i:i+10))
%     if abs(dis_LID(i)-aNum) >.1
%         dis_LID(i) = aNum
%     end
% end
% 
% 
% plot(dis_LID)
%         

%%
% % l1 = plot(fit_dis_LID);
% % hold on
% % l2 = plot(nshift_center_dist);
% % title(strcat(name,' LIDAR Distance vs Camera Center Distance'),'Interpreter', 'none')
% % legend({'Upper LIDAR Distance','Shifted Center_Dist','Original Center Dist'},'Interpreter', 'none')
% % xlabel('Index')
% % ylabel('Distance (m)')
% grid on
% %export_fig(PDF_NAME,'-transparent','-pdf','-append')
% hold off
% 
% 
%     grid on
%     C = linspecer(2);
%     
%     set(l1 , ...
%   'Color'           , C(1,:),...
%   'LineWidth'       , 2           );
% 
%     set(l2 , ...
%   'LineStyle'       , '-'      , ...
%   'LineWidth'       , 2,           ...
%   'Color'           , C(2,:)  );
%    
%     
%     hTitle  = title ('LIDAR and Camera Center Distances');
% 
%     set(gca,...
%     'Units','normalized',...
%     'YTick',0:1:12,...
%     'Position',[.15 .2 .75 .7],...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Helvetica');
% 
%     ylab = ylabel({'Distance (m)'},...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     xlab = xlabel('Index',...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     leg = legend('Upper LIDAR','Camera','location'...
%         ,'southwest');
%     
%     set([hTitle, xlab, ylab], ...
%         'FontName'   , 'Times');
%     
%        axis([0 90 0 12])
%     
%     set(gca, ...
%   'Box'         , 'off'     , ...
%   'TickDir'     , 'out'     , ...
%   'TickLength'  , [.02 .02] , ...
%   'XMinorTick'  , 'off'      , ...
%   'YMinorTick'  , 'off'      , ...
%   'YGrid'       , 'on'      , ...
%   'XColor'      , [.3 .3 .3], ...
%   'YColor'      , [.3 .3 .3], ...
%   'YTick'       , 0:1:12, ...
%   'LineWidth'   , 1         );
%  
% 
% 
% 
% 
% %% Plotting Center Dist
% figure
% title(strcat(name,' Center Distance'),'Interpreter', 'none')
% legend('Center Dist')
% xlabel('Index')
% ylabel('Distance (m)')
% export_fig(PDF_NAME,'-transparent','-pdf','-append')
% hold off

%% Plotting Average L1 & L2
plot(left_mean)
hold on
plot(right_mean)
title(strcat(name,' Camera L1 vs L2'),'Interpreter', 'none')
xlabel('Index')
ylabel('Distance (m)')
legend('L1','L2')
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')
hold off

%% Plotting camera theta_1 & theta_2
l1 = plot(theta_2*57.2958)
hold on
l2 = plot(theta_1*57.2958)
legend('\theta_2','\theta_1')
grid on
C = linspecer(5);

legend('\theta_2','\theta_1')

export_fig(PDF_NAME,'-transparent','-pdf','-append')
hold off


%%
    
% set(l1 , ...
% 'Color'           , C(2,:),...
% 'LineWidth'       , 2           );
% 
% set(gca,...
% 'Units','normalized',...
% 'YTick',0:1:12,...
% 'Position',[.15 .2 .75 .7],...
% 'FontUnits','points',...
% 'FontWeight','normal',...
% 'FontSize',16,...
% 'FontName','Helvetica');
% 
% ylab = ylabel({'Angle (Degrees)'},...
% 'FontUnits','points',...
% 'FontWeight','normal',...
% 'FontSize',16,...
% 'FontName','Times');
% 
% xlab = xlabel('Index',...
% 'FontUnits','points',...
% 'FontWeight','normal',...
% 'FontSize',16,...
% 'FontName','Times');
% 
%     leg = legend('\theta_2');
%     
%     set([xlab, ylab], ...
%         'FontName'   , 'Times');
%     
%    axis([0 90 -5 30])
%     
%     set(gca, ...
%   'Box'         , 'off'     , ...
%   'TickDir'     , 'out'     , ...
%   'TickLength'  , [.02 .02] , ...
%   'XMinorTick'  , 'off'      , ...
%   'YMinorTick'  , 'off'      , ...
%   'YGrid'       , 'on'      , ...
%   'XColor'      , [.3 .3 .3], ...
%   'YColor'      , [.3 .3 .3], ...
%   'YTick'       , -5:5:30, ...
%   'LineWidth'   , 1         );
%  
%   export_fig('Updated_Theta2','-transparent','-png')
%   hold off


% %% Plotting Center Dist
% figure
% title(strcat(name,' Center Distance'),'Interpreter', 'none')
% legend('Center Dist')
% xlabel('Index')
% ylabel('Distance (m)')
% export_fig(PDF_NAME,'-transparent','-pdf','-append')

%% Plotting Lidar theta_1 & theta_2
% plot(t1_LID)
% hold on
% plot(t2_LID)
% xlabel('Index')
% ylabel('Angle (rad)')
% title(strcat(name,'   LIDAR Theta_1 vs Theta_2'),'Interpreter', 'none')
% legend('\theta 1','\theta 2')
% grid on
% export_fig(PDF_NAME,'-transparent','-pdf','-append')
% hold off

% %% Plotting camera vs LIDAR theta_1
% plot(theta_1)
% hold on
% plot(t1_LID)
% xlabel('index')
% ylabel('Distance (m)')
% title(strcat(name,'   Camera vs LIDAR Theta_1'),'Interpreter', 'none')
% legend('Camera','LIDAR')
% grid on
% export_fig(PDF_NAME,'-transparent','-pdf','-append')
% hold off

% %% Plotting camera vs LIDAR theta_2
% plot(theta_2)
% hold on
% plot(t2_LID)
% xlabel('index')
% ylabel('Distance (m)')
% title(strcat(name,'   Camera vs LIDAR Theta_2'),'Interpreter', 'none')
% legend('Camera','LIDAR')
% grid on
% export_fig(PDF_NAME,'-transparent','-pdf','-append')
% hold off

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
plot(theta_2)
hold on
plot(theta_1)
plot(theta_path)
plot(left_mean/10)
plot(right_mean/10)
title(strcat(name,'   Camera Angles and Edge Distances'),'Interpreter', 'none')
legend('Rec \theta 2', 'Rec \theta 1', 'Rec \theta P', 'L1/10', 'L2/10')
grid on
export_fig(PDF_NAME,'-transparent','-pdf','-append')
hold off

%% Plotting calculated steering value

theta_1(end-10:end) = 0

    %time  = (.1:.1:length(theta_2)/10)';
    
    l1 = plot(theta_1)
    hold on
    l2 = plot(theta_2);
    l3 = plot(theta_path);
    l4 = plot(steer./8192);

    legend('\theta_1','\theta_2','\theta_P','Steering')
    
    export_fig(PDF_NAME,'-transparent','-pdf','-append')
    hold off
    
    %%
    
%     axis([0 90 -3 3])
%     
%     grid on
%     C = linspecer(4);
%     
%     set(l1 , ...
%   'Color'           , C(1,:),...
%   'LineWidth'       , 2           );
% 
%     set(l2 , ...
%   'LineStyle'       , '-'      , ...
%   'LineWidth'       , 2,           ...
%   'Color'           , C(2,:)  );
%     
%     set(l3 , ...
%   'LineStyle'       , '-'      , ...
%   'LineWidth'       , 2 ,          ...
%   'Color'          , C(3,:)        );
% 
%     set(l4 , ...
%   'LineStyle'       , '-'      , ...
%   'LineWidth'       , 2 ,          ...
%   'Color'          , C(4,:)        );
%     
%     set(gca,...
%     'Units','normalized',...
%     'YTick',-3:.5:3,...
%     'Position',[.15 .2 .75 .7],...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Helvetica');
% 
%     ylab = ylabel({'Amplitude, Radians--Steering Wheel Turns'},...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     xlab = xlabel('Index',...
%     'FontUnits','points',...
%     'FontWeight','normal',...
%     'FontSize',16,...
%     'FontName','Times');
% 
%     leg = legend('\theta_1','\theta_2','\theta_P','Steering Command','location'...
%         ,'southwest');
%     
%     set([xlab, ylab], ...
%         'FontName'   , 'Times');
%     
%     set(gca, ...
%   'Box'         , 'off'     , ...
%   'TickDir'     , 'out'     , ...
%   'TickLength'  , [.02 .02] , ...
%   'XMinorTick'  , 'off'      , ...
%   'YMinorTick'  , 'off'      , ...
%   'YGrid'       , 'on'      , ...
%   'XColor'      , [.3 .3 .3], ...
%   'YColor'      , [.3 .3 .3], ...
%   'YTick'       , -3:.5:3, ...
%   'LineWidth'   , 1         );
%  
%     export_fig('steering_plot_new','-transparent','-png')
%   hold off

  end
  
  close all
    
    %% TESTING CODE
%     
%         L = 2;
%     SPEED = 500;
%     delay = 110;
%     RMIN = 7.2;
%     
%     
%     dist_grad = ((SPEED /3600)*(1000/delay));

%     x_cam         = center_dist .* abs(cos(theta_1));
%     y_cam         = center_dist .* sin(theta_1);
%     
%     x_fwheel      = x_cam - L.*cos(theta_2);
%     y_fwheel      = y_cam - L.*sin(theta_2);
% 
%     y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
%     y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
%     
%     xdis = sqrt(L^2 - (y_cam_path - y_fwheel_path).^2);
%     
%     theta_path_calc = atan((y_cam_path - y_fwheel_path)./xdis); 
%     
%     angle_diff    = (theta_path_calc - theta_2);
%     
%     chan_f = (RMIN/dist_grad)*angle_diff;
%     
%     for i =1:length(chan_f)
%     if(chan_f(i) > 1)
% 	chan_f(i) = 1;
%     end
%     if(chan_f(i) < -1)
% 	chan_f(i) = -1;
%     end
%     end
%     
% 	new_steering = 24000*(chan_f);
    %%
    
L1 = left_mean;
L2 = right_mean;
    
W = 2.6;
y = 640;
theta_c = 55*pi/180;
rCoord = right_edge; % Recorded from test
lCoord = left_edge;  % Recorded from test

theta_n = acos((W^2+L2.^2-L1.^2)./(2.*L2.*W));
D = sqrt(W^2/4+L2.^2-L2.*W.*cos(theta_n));
theta_t = acos(((W/2)^2 + D.^2 - L2.^2)./(D*W));
theta_1_cal = pi/2 - theta_t;

x = (lCoord+rCoord)./2 - y;
theta_b = atan(x./y*tan(theta_c));
theta_2_cal = theta_1_cal+theta_b;

plot(nshift_theta_1)
hold on
plot(theta_1_cal)
plot(theta_2)
plot(theta_2_cal)
title('Angle Comparison Angled Righed')
legend('Recorded \theta 1','Calculated \theta 1',...
    'Recorded \theta 2','Calculated \theta 2')
xlabel('Index')
ylabel('Angel (rad)')
grid
% export_fig('Angle Comparison','-transparent','-pdf','-append')


% plot(D)
% hold on
% plot(center_dist)
% plot(dis_LID)

% plot(theta_1)
% hold on
% plot(theta_1_cal)


%     l = 2;
%     xc = center_dist.*cos(theta_1);
%     yc = center_dist.*sin(theta_1);
%     xf = xc - l.*cos(theta_2);
%     yf = yc - l.*sin(theta_2);
%     
%     new_b = -(xf.*tan(theta_2)-2.*yf)/(xf.^3);
%     new_a = (yf - new_b*xf.^3)./xf.^2;
%     
%     index=1:20;
%     plot(a(index))
%     hold on
%     plot(new_a(index))
%     legend('a','new_a')

%% Plotting difference between fith wheel location and path
% index = find(center_dist > 0);
% new_a = a(index);
% Number_Of_Unique_Paths = length(unique(new_a));
% 
% if (Number_Of_Unique_Paths == 1)
%     L = 2;
%     x_cam         = center_dist .* abs(cos(theta_1));
%     y_cam         = center_dist .* sin(theta_1);
%     
%     x_fwheel      = x_cam - L.*cos(theta_2);
%     y_fwheel      = y_cam - L.*sin(theta_2);
% 
%     y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
%     y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
%     
%     diff = abs(y_fwheel_path - y_fwheel);
%     plot(diff)
%     hold on
%     plot(center_dist)
%     grid on
%     legend('y_{fwheel} - y_{fwheel path}', 'center dist')
%     
% else
%     error('More than one path was calculated!')
% end



% %% theta_1 recalculation
% T1 = -0.1565;
% idx = 1;
% d = 9.8133;
% ax = 4;
% d_prime = sqrt(d^2 + ax^2 -2*d*ax*cos(T1))
% tB = acos((ax^2 + d_prime^2 - d^2)/(2*ax*d_prime))
% new_t1 = pi - tB

%% Plotting a and b coefficients to verify within tolerance
% figure
% index = 1:30;
% plot(a(index))
% hold on
% plot(abs(b(index)))
% amax = 1/7.2;
% bmax = 1/7.2^2;
% line1 = refline([0 amax]);
% line2 = refline([0 bmax]);
% line1.Color = 'r';
% line1.LineStyle = '--';
% line2.Color = 'r';
% line2.LineStyle = '--';
% legend('a','b')

% %% 
% find(abs(b)>bmax)
% 
% 
% %%  Plotting f_wheel locations
%     x_cam         = center_dist .* abs(cos(theta_1));
%     y_cam         = center_dist .* sin(theta_1);
%     
%     x_fwheel      = x_cam - L.*cos(theta_2);
%     y_fwheel      = y_cam - L.*sin(theta_2);
% 
%     y_cam_path    = a.*x_cam.^2 + b.*x_cam.^3;
%     y_fwheel_path = a.*x_fwheel.^2 + b.*x_fwheel.^3;
%     
%     plot(y_fwheel)
%     hold on
%     plot(y_fwheel_path)
%     hold on
%     plot(theta_2)
% 

