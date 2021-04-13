// ----------------------------------------------------------------------------
// minized_ftdi_serial.cpp
//
// 4/11/2021 D. W. Hawkins (dwh@caltech.edu)
//
// MiniZed FTDI FT2232H EEPROM serial string manipulation.
//
// This program enables:
//  - Read and display device strings
//  - Read and display EEPROM contents (and corresponding chars)
//  - Write new serial string
//
// ----------------------------------------------------------------------------
// Debugging
// ---------
//
// Windows 10 Cygwin experiences an unhandled exception that is not seen
// by this application, i.e., I think the application exits before getting
// to main(). Running strace on this application, I see
//
//   --- Process 21176, exception c0000005 at 0000000000cd5075
//   --- Process 21176 thread 28104 exited with status 0xc0000005
//   --- Process 21176 thread 22524 exited with status 0xc0000005
//
// But I have no explanation for why. I'll install the FTDI DLL under a
// different Windows 10 machine and see if I see the same error.
//
// ----------------------------------------------------------------------------
// Notes
// -----
//
// 1. Exclusive access to FTDI devices
//
//    The FTDI D2XX library will detect devices regardless of whether
//    other applications are currently accessing them. This application
//    requires exclusive access to the FTDI device, so make sure to exit
//    FT_PROG, the Digilent FTDI Configuration utility, or Vivado when
//    running this application. For example, if the Vivado hardware
//    manager is open and connected to the JTAG interface, this application
//    will fail with the following output:
//
//      $ ./minized_ftdi_serial
//      Found 2 devices
//      Error: FT_OpenEx returned 2
//
// 2. FTDI EEPROM access API
//
//    This application uses the FT_ReadEE and FT_WriteEE API calls, as the
//    the Xilinx/Avnet/Digilent customization within the EEPROM is not
//    preserved by the FTDI API calls FT_EE_Read and FT_EE_Program.
//    The FT2232H_EEPROM_Modify.cpp example from AN428 was modified to
//    read and update the serial number on a MiniZed. The application
//    changes the serial number, but it erases the Xilinx/Avnet/Digilent
//    data, which results in Vivado no longer recognizing the MiniZed.
//
// 3. EEPROM format
//
//    The EEPROM format has been reverse-engineered by the libFTDI developers.
//    For the purposes of this application, the bytes of importance are the
//    serial string offset bytes at byte offsets 0x12 and 0x13, and the
//    serial string which starts at byte offset 0xC0.
//
//    https://github.com/legege/libftdi/blob/master/doc/EEPROM-structure
//
//    See the notes below for the MiniZed default EEPROM image.
//
// ----------------------------------------------------------------------------
// References
// ----------
//
// [1] FTDI, "D2XX Programmer's Guide", Version 1.4, June 24, 2019.
//
//     p40: Section 4 intro has the comment that FT2232H device serial
//     strings are limited to 14 characters. The Digilent FTDI Configuration
//     tool limits the serial number to 12 characters, eg., '0123456789AB'.
//     This application uses a 12 character limit too.
//
// [2] FTDI Application Notes
//     https://ftdichip.com/document/application-notes/
//
//     AN_121: Accessing The EEPROM User Area Of FTDI Devices, v1.0, 2009-09-04
//     AN_123: How COM Ports Are Allocated On Driver Installation, v1.0, 2009-08-27
//
// [3] FTDI Technical Notes
//     https://ftdichip.com/document/technical-notes/
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

// Command-line processing
// (the C++ alternative is to use the Boost C++ library)
#include <getopt.h>

// ================================================================
// MiniZed default EEPROM image
// ================================================================
//
// FT2232H MiniZed device
// * Serial number = '1234-oj1'
// * The image was created by reading the EEPROM using FT_PROG
//   and then reformatting it as a C/C++ array
// * FT_PROG prints the ASCII view of the EEPROM content in
//   the opposite order to this application. The order used
//   in this application makes the strings easier to read!
//
// EEPROM with serial string '1234-oj1':
// 0000: 0801 0403 6010 0700 3280 0008 0000 0E9A .....`...2......
// 0008: 18A8 12C0 0000 0000 0056 0003 584A 7641 ........V...JXAv   -> 'Avnet'
// 0010: 656E 0074 694D 696E 655A 2064 3156 0000 net.MiniZed V1..   -> 'MiniZed V1'
// 0018: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0020: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0028: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0030: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0038: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0040: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0048: 0000 0000 0000 0000 0000 030E 0058 0069 ............X.i.   -> 'Xilinx'
// 0050: 006C 0069 006E 0078 0318 004A 0054 0041 l.i.n.x...J.T.A.   -> 'JTAG+Serial'
// 0058: 0047 002B 0053 0065 0072 0069 0061 006C G.+.S.e.r.i.a.l.
// 0060: 0312 0031 0032 0033 0034 002D 006F 006A ..1.2.3.4.-.o.j.   -> '1234-oj1'
// 0068: 0031 0302 0001 0000 0000 0000 0000 0000 1...............
// 0070: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0078: 0000 0000 0000 0000 0000 0000 0000 7295 ...............r
//
// EEPROM with serial string 'MiniZed01':
// 0000: 0801 0403 6010 0700 3280 0008 0000 0E9A .....`...2......
// 0008: 18A8 14C0 0000 0000 0056 0003 584A 7641 ........V...JXAv   -> 'Avnet'
// 0010: 656E 0074 694D 696E 655A 2064 3156 0000 net.MiniZed V1..   -> 'MiniZed V1'
// 0018: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0020: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0028: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0030: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0038: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0040: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0048: 0000 0000 0000 0000 0000 030E 0058 0069 ............X.i.   -> 'Xilinx'
// 0050: 006C 0069 006E 0078 0318 004A 0054 0041 l.i.n.x...J.T.A.   -> 'JTAG+Serial'
// 0058: 0047 002B 0053 0065 0072 0069 0061 006C G.+.S.e.r.i.a.l.
// 0060: 0314 004D 0069 006E 0069 005A 0065 0064 ..M.i.n.i.Z.e.d.   -> 'MiniZed01'
// 0068: 0030 0031 0302 0001 0000 0000 0000 0000 0.1.............
// 0070: 0000 0000 0000 0000 0000 0000 0000 0000 ................
// 0078: 0000 0000 0000 0000 0000 0000 0000 C0B5 ................
//
// From the libFTDI breakdown of the EEPROM content:
//
//    Byte  | Word   | Field
//    ------|--------|---------------
//    0x0E  | 0x07   | Offset Vendor
//    0x0F  | 0x07   | Len    Vendor
//    0x10  | 0x08   | Offset Product
//    0x11  | 0x08   | Length Product
//    0x12  | 0x09   | Offset Serial
//    0x13  | 0x09   | Length Serial
//
// The serial string byte offset is C0h, so word 0x0060, and the
// length is 0x14. The word at offset 0x0060 is 0x0314, which has
// the same length value as the word at offset 0x0009. The length
// value of 0x14 corresponds to 20-bytes or 10-words, which is the
// number of words to the next word with the pattern 0x03YY, where
// YY is the length in bytes of the next string.
//
std::vector<WORD> avnet_minized_default = {
	0x0801, 0x0403, 0x6010, 0x0700, 0x3280, 0x0008, 0x0000, 0x0E9A,
	0x18A8, 0x12C0, 0x0000, 0x0000, 0x0056, 0x0003, 0x584A, 0x7641,
	0x656E, 0x0074, 0x694D, 0x696E, 0x655A, 0x2064, 0x3156, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x030E, 0x0058, 0x0069,
	0x006C, 0x0069, 0x006E, 0x0078, 0x0318, 0x004A, 0x0054, 0x0041,
	0x0047, 0x002B, 0x0053, 0x0065, 0x0072, 0x0069, 0x0061, 0x006C,
	0x0312, 0x0031, 0x0032, 0x0033, 0x0034, 0x002D, 0x006F, 0x006A,
	0x0031, 0x0302, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7295
};

// ================================================================
// MiniZed FTDI EEPROM
// ================================================================
//
class minized_ftdi_eeprom {

	public:
		// Device handle
		FT_HANDLE m_handle;

		// EEPROM size in 16-bit words
		// 2kbit = 128 x 16-bit
		static const int EEPROM_WORDS = 128;

		// 16-bit EEPROM data
		std::vector<WORD> m_data;

	public:
		// Constructor
		minized_ftdi_eeprom();

		// Destructor
		~minized_ftdi_eeprom();

		// Read the EEPROM
		void read_eeprom();

		// Write the EEPROM
		void write_eeprom();

		// Write (update) the serial string
		void write_serial(std::string serial);

		// Read the serial string
		void read_serial(std::string &serial);

		// Calculate the checksum
		WORD calculate_checksum();

		// Update the checksum
		void update_checksum();

		// Print EEPROM contents
		void print_eeprom();
};

// Constructor
minized_ftdi_eeprom::minized_ftdi_eeprom()
{
	FT_STATUS status;
	FT_HANDLE handle;

	// Product description strings
	std::vector<std::string> dev_names{
		"Dual RS232-HS A", // Blank EEPROM
		"JTAG+Serial A"    // MiniZed
	};
	std::string dev_name;

	// FTDI devices list
	uint32_t dev_count;
	status = FT_CreateDeviceInfoList((DWORD *)&dev_count);
	if (status != FT_OK) {
		std::stringstream err;
		err << "FT_CreateDeviceInfoList returned " << (int)status;
		throw std::runtime_error(err.str());
	}

	// The MiniZed FT2232H device is detected as two devices (A+B)
	std::cout << "\nFound "
			  << dev_count
			  << " devices"
			  << std::endl;
	if (dev_count == 0) {
		std::stringstream err;
		err << "FTDI device count is zero!";
		throw std::runtime_error(err.str());
	}

	// Open the MiniZed board
	for (uint32_t i = 0; i < 2; i++) {
		dev_name = dev_names[i];
		status = FT_OpenEx(
			(PVOID)dev_name.c_str(),
			FT_OPEN_BY_DESCRIPTION,
			&handle);
		if (status == FT_OK) {
			break;
		}
	}
	if (status != FT_OK) {
		std::stringstream err;
		err << "FT_OpenEx returned " << (int)status;
		throw std::runtime_error(err.str());
	}
	std::cout << "\nSuccessfully opened '"
			  << dev_name
			  << "'"
			  << std::endl;

	// Save the handle
	m_handle = handle;
}

//Destructor
minized_ftdi_eeprom::~minized_ftdi_eeprom()
{
	if (m_handle) {
		FT_Close(m_handle);
		m_handle = 0;
	}
}

// Read the EEPROM
void minized_ftdi_eeprom::read_eeprom()
{
	FT_STATUS status;
	m_data.resize(EEPROM_WORDS);
	for (uint32_t i = 0; i < EEPROM_WORDS; i++) {
		status = FT_ReadEE(m_handle, i, &m_data[i]);
		if (status != FT_OK) {
			std::stringstream err;
			err << "FT_ReadEE returned " << (int)status;
			throw std::runtime_error(err.str());
		}
	}
}

// Write the EEPROM
void minized_ftdi_eeprom::write_eeprom()
{
	FT_STATUS status;
	for (uint32_t i = 0; i < EEPROM_WORDS; i++) {
		status = FT_WriteEE(m_handle, i, m_data[i]);
		if (status != FT_OK) {
			std::stringstream err;
			err << "FT_WriteEE returned " << (int)status;
			throw std::runtime_error(err.str());
		}
	}
}

// Write (update) the serial string (relative to the default EEPROM image)
void minized_ftdi_eeprom::write_serial(
	std::string serial
)
{
	// Default EEPROM contents
	// - Alternatively, m_data could be set in the constructor
	m_data = avnet_minized_default;

	// Update the serial string?
	if (serial.size() == 0) {
		return;
	}

	// EEPROM byte pointer
	uint8_t *bytes = (uint8_t *)(&m_data[0]);

	// Serial string offset and new string length
	uint32_t offset = bytes[0x12];
	uint32_t length = 2*serial.size()+2;
	//
	// Update the header string length
	bytes[0x13] = length;
	//
	// Update the serial string length
	bytes[offset] = length;
	//
	// Clear the bytes to the end of EEPROM
	memset(&bytes[offset+2],0,2*EEPROM_WORDS-offset-1);
	//
	// Update the serial string bytes
	for (uint32_t i = 0; i < (length-2)/2; i++) {
		bytes[offset + 2 + 2*i] = serial[i];
		bytes[offset + 3 + 2*i] = 0;
	}
	// Add the final 0x0302 0x0001 tag
	bytes[offset+length]   = 02;
	bytes[offset+length+1] = 03;
	bytes[offset+length+2] = 01;
	bytes[offset+length+3] = 00;

	// Update the checksum
	update_checksum();
}

// Read the EEPROM serial string
void minized_ftdi_eeprom::read_serial(std::string &serial)
{
	// Serial string location
	uint8_t *bytes = (uint8_t *)(&m_data[0]);
	uint32_t offset = bytes[0x12];
	uint32_t length = bytes[0x13];

	// Extract the serial string
	std::stringstream serialstream;
	for (uint32_t i = 0; i < (length-2)/2; i++) {
		serialstream << bytes[offset + 2 + 2*i];
	}
	serial = serialstream.str();
}

// Calculate the checksum
WORD minized_ftdi_eeprom::calculate_checksum()
{
	WORD checksum = 0xAAAA;
	for (uint32_t i = 0; i < (EEPROM_WORDS-1); i++) {
		checksum ^= m_data[i];
		checksum = (checksum << 1) | (checksum >> 15);
	}
	return checksum;
}

// Update the checksum
void minized_ftdi_eeprom::update_checksum()
{
	m_data[EEPROM_WORDS-1] = calculate_checksum();
}

// Print the EEPROM contents
void minized_ftdi_eeprom::print_eeprom()
{
	uint32_t rows = EEPROM_WORDS/8;
	std::cout << "\nEEPROM contents:" << std::endl;
	for (uint32_t i = 0; i < rows; i++) {
		// Address
		std::cout << std::setw(4)
		          << std::setfill('0')
				  << std::hex
				  << std::uppercase
				  << 8*i << ":";

		// 8 x 16-bit hex values
		for (uint32_t j = 0; j < 8; j++) {
			std::cout << " "
					  << std::setw(4)
					  << std::setfill('0')
					  << std::hex
					  << std::uppercase
					  << m_data[8*i+j];
		}

		// 16 x 8-bit character values
		std::cout << " ";
		for (uint32_t j = 0; j < 8; j++) {
			char lsb =  m_data[8*i+j]       & 0xFF;
			char msb = (m_data[8*i+j] >> 8) & 0xFF;
			std::cout << ((lsb >= 32) ? lsb : '.')
					  << ((msb >= 32) ? msb : '.');
		}
		std::cout << std::endl;
	}
	std::cout << std::dec << std::endl;

	// Calculate the checksum
	WORD checksum = calculate_checksum();
	std::cout << "EEPROM   checksum: "
			  << std::setw(4) << std::setfill('0')
			  << std::hex << std::uppercase
			  << m_data[EEPROM_WORDS-1]
			  <<  std::endl;
	std::cout << "Expected checksum: "
			  << std::setw(4) << std::setfill('0')
			  << std::hex << std::uppercase
			  << checksum
			  << std::endl;
	if (checksum == m_data[EEPROM_WORDS-1]) {
		std::cout << "\nChecksums match!" << std::endl;
	} else {
		std::cout << "\nError: Checksum mismatch!" << std::endl;
	}
}

// ================================================================
// Help
// ================================================================
//
static void help()
{
	std::cout <<
		 "\n"\
		 "MiniZed FTDI EEPROM serial string tool\n"\
		 "--------------------------------------\n\n"\
		 "Usage: minized_ftdi_serial [options]\n"\
		 "  -h | --help            Help (this message)\n"\
		 "\n"\
		 "EEPROM contents:\n"\
		 "  -e | --eeprom          Read and print the EEPROM.\n"\
		 "\n"\
		 "Serial string read:\n"\
		 "  -r | --read            Read the serial string.\n"\
		 "\n"\
		 "Serial string write:\n"\
		 "  -w | --write <string>  Write a new serial string.\n"\
		 "\n"\
		 "Serial string write:\n"\
		 "  -d | --default         Write the default EEPROM image.\n"\
		 "\n" << std::endl;
}

// ================================================================
// Main Application
// ================================================================
//
int main (int argc, char **argv)
{
	// Command line strings
	std::string serial;
	bool print = false;
	bool read = false;
	bool write = false;

	// Command-line argument parsing
	int opt;
	int option_index = 0;
	static struct option long_options[] = {
		/* Long and short options */
		{"default",   0, 0, 'd'},
		{"eeprom",    0, 0, 'e'},
		{"help",      0, 0, 'h'},
		{"read",      0, 0, 'r'},
		{"write",     1, 0, 'w'},
		{0, 0, 0, 0}
	};
	while ((opt = getopt_long(
		argc, argv, "dehrw:", long_options, &option_index)) != -1) {
		switch (opt) {
			/* Long and short options */

			case 'd':
				write = true;
				break;

			case 'h':
				help();
				return -1;

			case 'e':
				print = true;
				break;

			case 'r':
				read = true;
				break;

			case 'w':
				write = true;
				serial = optarg;
				break;

			default:
				help();
				return -1;
		}
	}

	// Check for required arguments
	if (!(print || read || write || serial.size())) {
		help();
		return -1;
	}

	// Serial string length check
	if (serial.size() > 12) {
		std::cout << "The serial string must be 12 characters or less"
		          << std::endl;
		return 0;
	}

	try {
		// Perform the write first, so that if read arguents are
		// also provided, the new EEPROM content is read.

		// EEPROM write
		if (write) {
			minized_ftdi_eeprom minized;
			minized.write_serial(serial);
			minized.write_eeprom();
		}

		// EEPROM read
		//  - print the EEPROM contents or
		//  - read the serial string
		if (read || print) {
			minized_ftdi_eeprom minized;
			minized.read_eeprom();

			// Print the EEPROM contents
			if (print) {
				minized.print_eeprom();
			}

			// Read the serial string from the EEPROM
			if (read) {
				std::string eeprom_serial;
				minized.read_serial(eeprom_serial);
				std::cout << "\nSerial string = '"
						  << eeprom_serial
						  << "'"
						  << std::endl;
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Error: Unknown exception!" << std::endl;
	}
	return 0;
}

