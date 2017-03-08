//
//  constants.cpp
//  
//
//  Created by Christian Heussy on 3/6/17.
//
//



steering_proportional

// CAN //

// Message Identifiers
steering_command_ID = 0x18FFEF27;

// VDC2 - vehicle acceleration signal
long VDC2_ID = 0x18F00927;
VDC2_DLC = 8;
VDC2_FLAG = canMSG_EXT;

// EBC1 - brake control information
long EBC1_ID = 0x18F00127;
EBC1_DLC = 08;
EBC1_FLAG = canMSG_EXT;
