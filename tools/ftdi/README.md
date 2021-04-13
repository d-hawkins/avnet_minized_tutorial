# Avnet MiniZed FTDI utilities

4/10/2021 D. W. Hawkins (dwh@caltech.edu)

# FTDI D2XX Utilities

File                    | Description
------------------------|-----------
ftdi_list_devices.cpp   | List FTDI devices
minized_ftdi_serial     | MiniZed FTDI EEPROM utility (can change the serial string)

# Example

Under Windows 10 using a Cygwin console with a single MiniZed board attached.

List the FTDI devices:

~~~
$ ./ftdi_list_devices
2 FTDI devices found
Device 0
  Flags        2
  Type         6
  ID           67330064
  LocID        4417
  SerialNumber MiniZed01A
  Description  JTAG+Serial A
Device 1
  Flags        2
  Type         6
  ID           67330064
  LocID        4418
  SerialNumber MiniZed01B
  Description  JTAG+Serial B
~~~

Run the MiniZed FTDI application without any arguments:

~~~
MiniZed FTDI EEPROM serial string tool
--------------------------------------

Usage: minized_ftdi_serial [options]
  -h | --help            Help (this message)

EEPROM contents:
  -e | --eeprom          Read and print the EEPROM.

Serial string read:
  -r | --read            Read the serial string.

Serial string write:
  -w | --write <string>  Write a new serial string.

Serial string write:
  -d | --default         Write the default EEPROM image.
~~~

Use the MiniZed FTDI application to read the EEPROM and serial string:

~~~
$ ./minized_ftdi_serial -e -r

Found 2 devices

Successfully opened 'JTAG+Serial A'

EEPROM contents:
0000: 0801 0403 6010 0700 3280 0008 0000 0E9A .....`...2......
0008: 18A8 14C0 0000 0000 0056 0003 584A 7641 ........V...JXAv
0010: 656E 0074 694D 696E 655A 2064 3156 0000 net.MiniZed V1..
0018: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0020: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0028: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0030: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0038: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0040: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0048: 0000 0000 0000 0000 0000 030E 0058 0069 ............X.i.
0050: 006C 0069 006E 0078 0318 004A 0054 0041 l.i.n.x...J.T.A.
0058: 0047 002B 0053 0065 0072 0069 0061 006C G.+.S.e.r.i.a.l.
0060: 0314 004D 0069 006E 0069 005A 0065 0064 ..M.i.n.i.Z.e.d.
0068: 0030 0031 0302 0001 0000 0000 0000 0000 0.1.............
0070: 0000 0000 0000 0000 0000 0000 0000 0000 ................
0078: 0000 0000 0000 0000 0000 0000 0000 C0B5 ................

EEPROM   checksum: C0B5
Expected checksum: C0B5

Checksums match!

Serial string = 'MiniZed01'
~~~


