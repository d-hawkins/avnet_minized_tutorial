/* ----------------------------------------------------------------------------
 * uboot_crc32.c
 *
 * 6/9/2019 D. W. Hawkins (dwh@caltech.edu)
 *
 * U-Boot compatible CRC32 calculation.
 *
 * ----------------------------------------------------------------------------
 * U-Boot Example
 * --------------
 *
 * crc32_data.bin was created to contain incrementing bytes 00 to FF
 * and the file was downloaded to U-Boot using the kermit protocol.
 *
 * At the U-Boot terminal, enter the command 'loadb' and then send the
 * binary data file to U-Boot using TeraTerm File->Transfer->Kermit->Send.
 *
 * Prior to downloading the file you can clear an area of memory as shown
 * by the console output below.
 *
 * Zynq> mw.b 10000000 0 100
 * Zynq> md.b 10000000 100
 * 10000000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 10000090: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * 100000f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
 * Zynq> loadb
 * ## Ready for binary (kermit) download to 0x10000000 at 115200 bps...
 * ## Total Size      = 0x00000100 = 256 Bytes
 * ## Start Addr      = 0x10000000
 * Zynq> md.b 10000000 100
 * 10000000: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    ................
 * 10000010: 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f    ................
 * 10000020: 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f     !"#$%&'()*+,-./
 * 10000030: 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f    0123456789:;<=>?
 * 10000040: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f    @ABCDEFGHIJKLMNO
 * 10000050: 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f    PQRSTUVWXYZ[\]^_
 * 10000060: 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f    `abcdefghijklmno
 * 10000070: 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f    pqrstuvwxyz{|}~.
 * 10000080: 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f    ................
 * 10000090: 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f    ................
 * 100000a0: a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af    ................
 * 100000b0: b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf    ................
 * 100000c0: c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf    ................
 * 100000d0: d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df    ................
 * 100000e0: e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef    ................
 * 100000f0: f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff    ................
 * Zynq> crc32 10000000 100
 * crc32 for 10000000 ... 100000ff ==> 29058c73
 * Zynq> crc32 10000000 80
 * crc32 for 10000000 ... 1000007f ==> 24650d57
 * Zynq> crc32 10000000 10
 * crc32 for 10000000 ... 1000000f ==> cecee288
 * Zynq>
 *
 * The same CRC calculations are performed by this application:
 *
 * $ ./uboot_crc32 -f crc32_data.bin
 * Size = 0x00000100 (256) bytes
 * CRC  = 0x29058C73 (688229491)
 *
 * $ ./uboot_crc32 -f crc32_data.bin -s 0x80
 * Size = 0x00000080 (128) bytes
 * CRC  = 0x24650D57 (610602327)
 *
 * $ ./uboot_crc32 -f crc32_data.bin -s 0x10
 * Size = 0x00000010 (16) bytes
 * CRC  = 0xCECEE288 (3469664904)
 *
 * This application can be used to calculate the CRC of a QSPI flash
 * image to compare to a binary file. Comparing the CRC32 values is
 * much faster than transferring the file to U-Boot via Kermit to
 * perform a byte-by-byte compare (or copying data from U-Boot to file).
 *
 * ----------------------------------------------------------------------------
 * MiniZed QSPI Flash Image Example
 * --------------------------------
 *
 * The three MiniZed QSPI flash image CRCs are:
 *
 * $ ./uboot_crc32 -f c:/temp/minized/flash_only_boot_7007S.bin
 * Size = 0x00FDA5F4 (16623092) bytes
 * CRC  = 0x86771B07 (2255952647)
 *
 * $ ./uboot_crc32 -f c:/temp/minized/flash_fallback_7007S.bin
 * Size = 0x00FC0BE4 (16518116) bytes
 * CRC  = 0x005B7B3C (5995324)
 *
 * $ ./uboot_crc32 -f c:/temp/minized/smallboot.bin
 * Size = 0x00FC0BE4 (16518116) bytes
 * CRC  = 0x005B7B3C (5995324)
 *
 * The U-Boot power-on-reset message
 *
 *    U-Boot 2016.07 (Jul 27 2017 - 21:56:59 -0700)
 *
 * corresponds to the image flash_only_boot_7007S.bin.
 *
 * The MiniZed 16MB flash contents can be copied to DDR memory using
 *
 * Zynq> sf probe 0 0
 * SF: Detected N25Q128 with page size 256 Bytes, erase size 64 KiB, total 16 MiB
 * Zynq> sf read 10000000 0 1000000
 * device 0 whole chip
 * SF: 16777216 bytes @ 0x0 Read: OK
 *
 * The CRC can then be calculated using
 *
 * Zynq> crc32 10000000 FDA5F4
 * crc32 for 10000000 ... 10fda5f3 ==> 86771b07
 *
 * The U-Boot CRC matches the CRC above.
 *
 * This was on a MiniZed that I had reprogrammed.
 *
 * Use Vivado 2019.1 and program the fallback image.
 *
 * The U-Boot power-on-reset message changes to
 *
 *   U-Boot 2017.01 (Mar 22 2018 - 23:33:56 -0700)
 *
 * Zynq> sf probe 0 0 0
 * SF: Detected n25q128 with page size 256 Bytes, erase size 64 KiB, total 16 MiB
 * Zynq> sf read 10000000 0 1000000
 * device 0 whole chip
 * SF: 16777216 bytes @ 0x0 Read: OK
 * Zynq> crc32 10000000 FC0BE4
 * crc32 for 10000000 ... 10fc0be3 ==> 005b7b3c
 *
 * The U-Boot CRC matches that calculated above.
 *
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

/* ----------------------------------------------------------------------------
 * Local Function Declarations
 * ----------------------------------------------------------------------------
 */
static void show_usage();
static uint32_t crc32(uint8_t *data, uint32_t len);
static void bit_reverse_bytes(uint8_t *data, uint32_t len);
static uint32_t bit_reverse_crc32(uint32_t crc);

/* ----------------------------------------------------------------------------
 * Main Application
 * ----------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	char *filename = NULL;
	struct stat st;
	FILE *fp;
	uint8_t *buffer;
	uint32_t file_size;
	uint32_t read_size;
	uint32_t size = 0;
	uint32_t status;
	uint32_t crc;
	uint32_t pad = 0xFF;

	/* Command-line argument parsing */
	int opt;
	int option_index = 0;
	static struct option long_options[] = {
		/* Long and short options */
		{"file",     1, 0, 'f'},
		{"help",     0, 0, 'h'},
		{"pad",      1, 0, 'p'},
		{"size",     1, 0, 'a'},
		{0, 0, 0, 0}
	};
	while ((opt = getopt_long(
		argc, argv, "f:hp:s:", long_options, &option_index)) != -1) {
		switch (opt) {
			/* Long and short options */

			case 'f':
				filename = optarg;
				break;

			case 'h':
				show_usage();
				return -1;

			case 'p':
				if ( (optarg[0] == '0') &&
					((optarg[1] == 'x') || (optarg[1] = 'X')) ) {
					/* scan hex */
					status = sscanf(optarg, "%x", &pad);
				} else {
					/* scan decimal */
					status = sscanf(optarg, "%d", &pad);
				}
				if (status < 0) {
					printf("Error: invalid size argument\n");
					return -1;
				}
				break;

			case 's':
				if ( (optarg[0] == '0') &&
					((optarg[1] == 'x') || (optarg[1] = 'X')) ) {
					/* scan hex */
					status = sscanf(optarg, "%x", &size);
				} else {
					/* scan decimal */
					status = sscanf(optarg, "%d", &size);
				}
				if (status < 0) {
					printf("Error: invalid size argument\n");
					return -1;
				}
				break;

			default:
				show_usage();
				return -1;
		}
	}

	/* Filename is required */
	if ((filename == NULL) || (strlen(filename) == 0)) {
		printf("Error: Filename required!\n");
		show_usage();
		return 0;
	}

	/* File size */
	if (stat(filename, &st) != 0) {
		printf("Error: file stat failed!\n");
		return -1;
	}
	file_size = (uint32_t)(st.st_size);

	/* Default size */
	if (size == 0) {
		size = file_size;
	}

	printf("File size = 0x%.8X (%u) bytes\n", file_size, file_size);
	printf("CRC size  = 0x%.8X (%u) bytes\n", size, size);

	/* Buffer */
	buffer = (uint8_t *)malloc(size*sizeof(uint8_t));
	if (buffer == NULL) {
		printf("Error: malloc failed!\n");
		return -1;
	}
	memset(buffer, pad, size);

	/* Open the file */
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Error: file open failed!\n");
		return -1;
	}

	/* Read the file */
	read_size = (size < file_size) ? size : file_size;
	status = (uint32_t)fread(buffer, sizeof(char), read_size, fp);
	if (status != read_size) {
		printf("Error: read failed!\n");
		return -1;
	}

	/* Close the file */
	fclose(fp);

	/* Bit reverse the bytes */
	bit_reverse_bytes(buffer, size);

	/* Calculate the CRC */
	crc = crc32(buffer, size);

	/* Bit reverse the CRC */
	crc = bit_reverse_crc32(crc);

	printf("CRC       = 0x%.8X (%u)\n", crc, crc);

	/* Free the buffer */
	free(buffer);
	return 0;
}

/* ----------------------------------------------------------------------------
 * Local Function Implementations
 * ----------------------------------------------------------------------------
 */
/* Usage */
static void show_usage()
{
	printf(
		 "\n"\
		 "CRC32 calculation (compatible with U-Boot)\n"\
		 "------------------------------------------\n\n"\
		 "Usage: uboot_crc32 [options]\n"\
		 "  -h | --help                Help (this message)\n"\
		 "  -f | --file <filename>     Filename to check\n"\
		 "  -s | --size <size>         Size\n"\
		 "  -p | --pad <value>         Pad byte value (default 0xFF)\n"\
		 "\n"\
		 "The size argument can be smaller or larger than the file size.\n"\
		 "If size is larger than the file, the pad value is used.\n"\
		 "The pad bytes value defaults to 0xFF, to emulate empty flash\n"\
		 "bytes following an image."\
		 "\n");
}

/* CRC32 lookup table for polynomial 0x04C11DB7
 * Calculated using the CRC tool: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
 *    CRC32
 *     - Input reflected
 *     - Result reflected
 *     - Polynomial      0x04C11DB7
 *     - Initial value   0xFFFFFFFF
 *     - Final XOR value 0xFFFFFFFF
 */
static uint32_t crc_table[256] = {
0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4};

/* This loop gives the same results as Sunshine2k for input not
 * reflected and results not reflected
 */
uint32_t crc32 (uint8_t *data, uint32_t len)
{
	/* Initial value */
	uint32_t crc = 0xffffffff;

	for (uint32_t i=0; i < len; i++) {
		crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data++) & 0xff];
	}

	/* Final XOR = 0xFFFFFFFF */
	crc = ~crc;
	return crc;
}

/* Implement the Sunshine2k 'input reflected' bit-swap for input bytes */
void bit_reverse_bytes(uint8_t *data, uint32_t len)
{
	uint8_t byte, swap;
	for (uint32_t j = 0; j < len; j++) {
		byte = data[j];
		swap = 0;
		for (uint32_t i = 0; i < 8; i++) {
			swap |= ((byte >> i) & 1) << (7-i);
		}
		data[j] = swap;
	}
}

/* Implement the Sunshine2k 'result reflected' bit-swap for the 32-bit CRC */
uint32_t bit_reverse_crc32(uint32_t crc)
{
	uint32_t swap = 0;
	for (uint32_t i = 0; i < 32; i++) {
		swap |= ((crc >> i) & 1) << (31-i);
	}
	return swap;
}


