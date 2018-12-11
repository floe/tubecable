#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*unsigned char nullscrambling1[] = { 0x57, 0xCD, 0xDC, 0xA7, 0x1C, 0x88, 0x5E, 0x15, 0x60, 0xFE, 0xC6, 0x97, 0x16, 0x3D, 0x47, 0xF2 };
unsigned char nullscrambling2[] = { 0x47, 0x3D, 0x16, 0x97, 0xC6, 0xFE, 0x60, 0x15, 0x5E, 0x88, 0x1C, 0xA7, 0xDC, 0xB7, 0x6F, 0xF2 };

unsigned char capkey[] = { 0xb3,0x12,0x4d,0xc8,0x43,0xbb,0x8b,0xa6,0x1f,0x03,0x5a,0x7d,0x09,0x38,0x25,0x1f };*/

unsigned char nullkey[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x18,0xF0 };

unsigned char key[16];

uint8_t  keybuffer[0x11000];
uint16_t ofsbuffer[0x1000];

// x^12+x^11+x^3+x^2+x+1 = 0001 1000 0000 1111 = 0x180F
int dl_crc12( unsigned char* data, int len) {
	int rem = 0;
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < 8; j++) {
			rem = (rem << 1) | ((data[i] >> j) & 0x01);
			if (rem & 0x1000) rem = rem ^ 0x180F;
		}
	}
	return rem;
}


void dl_generate_key( uint8_t key[0x11000], uint16_t map[0x1000] ) {

  int coeffs[0x20];
  int count = 0;

  // loop1:
  for (int i = 0; i < 0x20; i++) {

    int tmp = 1 << i;
    if (!(tmp & 0x0829)) continue;

    coeffs[count] = i;
    count++;
  }

  int val = 0x01;

  // loop2:
  for (int i = 0; i < 0x11000; i++) {

    key[i] = val;
    if (i < 0x1000) map[val] = i;

    // loop3:
    for ( int j = 8; j > 0; j--) {

      int res = 0;

			// loop4:
      for (int k = 0; k < count; k++) {
        int coeff = coeffs[k];
        coeff = val >> coeff;
        res = res ^ coeff;
      }

      res = res & 1;
      res = res ^ (2*val);
      val = res & 0xFFF;
    }
  }
}

int main( int argc, const char* argv[] ) {


	printf("dldecrypt 0.1 - strip displaylink on-wire encryption\n");

	if ((argc < 3) || (strlen(argv[1]) != 32)) {
		printf("usage: %s key_in_hex file1 file2 file3...\n",argv[0]);
		exit(1);
	}

	dl_generate_key( keybuffer, ofsbuffer );

	/*FILE* f = fopen("synthkey.raw","w+");
	fwrite(keybuffer,1,0x11000,f);
	fclose(f);

	FILE* f2 = fopen("map.raw","w+");
	fwrite(ofsbuffer,2,0x1000,f2);
	fclose(f2);*/

	/*printf("%x\n",dl_crc12(nullscrambling1,16));
	printf("%x\n",dl_crc12(nullkey,16));
	printf("%x\n",dl_crc12(nullscrambling2,16));*/

	for (int i = 0; i < 16; i++) {
		const char tmp[3] = { argv[1][2*i], argv[1][2*i+1], 0 };
		if ( sscanf( tmp, "%hhx", &(key[i]) ) == 1 ) continue;
		printf("error in hex string\n"); exit(2);
	}

	int crc = dl_crc12(key,16);
	int offset = ofsbuffer[crc];
	printf("key crc: 0x%x, start offset: 0x%x\n",crc,offset);

	for (int i = 2; i < argc; i++) {

		char outfile[1024];
		snprintf( outfile, sizeof(outfile), "%s.dec", argv[i] );

		printf("processing file #%d: %s -> %s\n",i-1,argv[i],outfile);

		FILE* in  = fopen( argv[i], "r" );
		FILE* out = fopen( outfile, "w" );

		if (!in || !out) {
			printf("error opening file #%d\n",i-1);
			exit(3);
		}

		while (!feof(in)) {
			unsigned char buffer[4095];
			int size = fread( buffer, 1, sizeof(buffer), in );
			for (int j = 0; j < size; j++) buffer[j] = buffer[j] ^ keybuffer[j+offset];
			offset += size;
			offset %= 4095;
			fwrite( buffer, 1, size, out );
		}

		fclose(in);
		fclose(out);
	}
}

