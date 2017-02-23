Preliminary pseudocode of system integration code:

#include necessary functions
#define constants

struct track {			// struct for keeping a memory of previous data
	long cur_x[2];		// current x position of tractor axes [front, back]
	long cur_y[2];		// current y position of tractor axes
	long closest_obj;	// distance to closest object (for obtacle detection)
	long trailer[4];	// position of chosen trailer [distance, theta_1, pixel_x, pixel_y]  store chosen trailer
	long slope;			// slope of the trailer surface
} memory;


// will still need to relate driver choice to LIDAR


int main()
{
                            // Initialize variables:

	long y[RES];			// array for holding the path function, y is in meters
	int safe = 1;			// flag that when = 0 signals the system to stop.
	int n_done = 1;			// when = 1 means we are not coupled.
	int speed;				// set these values so that initially nothing happens when the threads are created.
	int height;
	int steer;
    int brake;
	int speed_prev;			// holds previous speed when we stop for safety reasons.
	int king_det = 0;		// flag to signal when kingpin is detected.
	long st_coeff;			// coefficient for steering control
	long dist_grad;			// distance per index of the x[], so that we can convet between x-index value and x-distance for easier comparison
	long y_exp[2];			// expected values of position based on the path
	int ind_x[2];			// index of x-position corresponding to y position

	
					// Initialize camera, CAM communication channels, LIDAR.


					// Check for error messages, sensor calibration.
    

	chooseTrailer(memory, y, &safe);	// Function that does the first detection, prompts driver to choose trailer
												// saves the location of that trailer to memory and derives the initial path.
												// Can also ask driver if path is clear and safe. Prompt driver for camera
    


					// Start controlling the tractor
                    // Each thread will be used to send commands at specified rate, updating

	thread t1(transmission, speed);	// Start independent threads for moving the truck			
	t1.detach();					// with appropriate arguments
	thread t2(suspension, height);		
	t2.detach();
	thread t3(steering, steer);		//Mutex to lock values, or switching memory locations
	t3.detach();
    thread t4(brake, brake);
    t4.detach();
    thread t5(read lock, read lock);
    t5.detach();
    
    
    

					// Feedback Loop
	dist_grad = memory.cur_x[1] / RES;				// Set distance gradient
    
    
    // CAN CONTROL

    // STEERING //
    // read path slope
    // read truck slope from sensors
    steering_error = truck_slope - path_slope;
    new_steering_command = proportional_gain * steering_error;
    
    command = new_steering_command;
    // update command value used in steering thread
    
    int apply_brake(brake_demand){
        // if apply brake called, send brake message on bus
        if (brake_demand = true)
        breake_on =true // Boolean value used by thread
            }
    
    
    
	while (n_done)
	{
		// Check for obstacles
		if (!(safe && (memory.closest_obj < memory.trailer[0]))  // If safety flag is triggered, or distance to closest object is smaller than 
		{														// distance to trailer, reduce speed to 0 and apply brakes.
			speed_prev = speed;
			speed = 0;
			brake();
			while (!(memory.closest_obj < memory.trailer[0]))
			{
				wait();
				scan(memory);
			}
			//here we could ask the driver if it is safe to proceed.
			safe = !safe;
			speed = speed_prev;
		}

		scan_hor(memory);					// scan with camera and horizontal LIDAR
		scan_ver(&king_det, &height);		// scan with vertical LIDAR, set the height for suspension

		if (king_det)						// if detected kingpin, slow down and drive straight until coupled
		{
			steer = 0;
			speed = speed_reduced;
			thread t4(checkCoupling, std::ref(n_done));
			t4.detach
		}
		else{
			ind_x[0] = (int)round(memory.cur_x[0] / dist_grad) - 1;
			ind_x[1] = (int)round(memory.cur_x[1] / dist_grad) - 1;
			y_exp[0] = y[ind_x[0]];
			y_exp[1] = y[ind_x[1]];
			if ((y_exp[1] - memory.cur_y[1]) > LIMIT)			// Check if we are way off path
			{
				path(memory,y);
				dist_grad = memory.cur_x[1] / RES;				// Set distance gradient
				ind_x[0] = (int)round(memory.cur_x[0] / dist_grad) - 1;
				ind_x[1] = (int)round(memory.cur_x[1] / dist_grad) - 1;
				y_exp[0] = y[ind_x[0]];
				y_exp[1] = y[ind_x[1]];
			}
			steer = st_coeff * (y_exp[0] - y_exp[1] - memory.cur_y[0] + memory.cur_y[1]) / (memory.cur_x[0] - memory.cur_x[1]);
			// This just relates the steering as proportional to the difference in slopes between the actual slope of the tractor position and the expected slope at that position.
			// We can modify this function so that the coefficient of proportionality becomes adjusted with every iteration, or to include the second derivative in the
			// calculation.
		}
	}
		
	//terminate threads
            
    // Tug test
            
    // Create thread to check for brake press. If driver presses brake, prompt driver to reinitialize system
            
	//congratulate driver for surviving
	
	return 0;
	}


functions used:

void chooseTrailer(memory, x, y, &safe)
// This function runs in the beginning, it will in itself run a scan_hor function to scan for trailers, then ask the driver to choose the one he wants.
// It will also prompt him to do the necessary steps for us to take control of the truck. The function will then call the path function to calculate the path. Asking if it is 
// safe to proceed is optional.

transmission, steering, brake, and suspension will control the tractor by sending signals at constant intervals(we can add the ability to return values to the main function for comparison)

void scan_hor(memory)
// Reads from the LIDAR and camera, uses memory.trailer[] to keep track of the driver-chosen trailer, then modifies all the values in memory based on detection
// this function does the mathematical calculation, and compares the data from the sensors and camera

void scan_ver(&kind_det, &height)
// Reads from vertical LIDAR, sets the height for suspension and triggers a flag if kingpin is detected

checkCoupling // checks if the lock on the fifth wheel is engaged, triggers the flag that ends the loop and the program

void path(memory, y) // calculates the path function