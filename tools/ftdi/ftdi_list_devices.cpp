// ----------------------------------------------------------------------------
// ftdi_list_devices.cpp
//
// 12/10/2018 D. W. Hawkins (David.W.Hawkins@jpl.nasa.gov)
//
// List the FTDI devices attached via USB.
//
// ----------------------------------------------------------------------------


// Windows types used by the FTDI D2XX direct access API
#ifdef __linux__
#include "WinTypes.h"
#else
#include <windows.h>
#endif
#include <unistd.h> // usleep()

// FTDI D2XX direct access API
#include "ftd2xx.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

// ================================================================
// Main Application
// ================================================================
//
int main (int argc, char **argv)
{
	try {
		FT_STATUS status;
		DWORD num_devices;
		status = FT_CreateDeviceInfoList(&num_devices);
		if (status != FT_OK) {
			std::stringstream err;
			err << "FT_CreateDeviceInfoList returned " << (int)status;
			throw std::runtime_error(err.str());
		}
		if (num_devices < 1) {
			std::cout << "No FTDI devices detected" << std::endl;
			return 0;
		}
		std::cout << num_devices << " FTDI devices found" << std::endl;

		std::vector<FT_DEVICE_LIST_INFO_NODE> dev_info(num_devices);
		status = FT_GetDeviceInfoList(&dev_info[0], &num_devices);
		if (status != FT_OK) {
			std::stringstream err;
			err << "FT_GetDeviceInfoList returned " << (int)status;
			throw std::runtime_error(err.str());
		}
		for (uint32_t i = 0; i < num_devices; i++) {
			std::cout << "Device " << i
			          << "\n  Flags        " << dev_info[i].Flags
			          << "\n  Type         " << dev_info[i].Type
			          << "\n  ID           " << dev_info[i].ID
			          << "\n  LocID        " << dev_info[i].LocId
			          << "\n  SerialNumber " << dev_info[i].SerialNumber
			          << "\n  Description  " << dev_info[i].Description
			          << std::endl;
		}


	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}

