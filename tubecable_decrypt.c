#include "tubecable.h"

unsigned char key[16];

int main( int argc, const char* argv[] ) {

	printf("dldecrypt 0.1 - strip displaylink on-wire encryption\n");

	if ((argc < 3) || (strlen(argv[1]) != 32)) {
		printf("usage: %s key_in_hex file1 file2 file3...\n",argv[0]);
		exit(1);
	}

	dl_crypt_generate_key( dl_crypt_keybuffer, dl_crypt_ofsbuffer );

	// read key
	for (int i = 0; i < 16; i++) {
		const char tmp[3] = { argv[1][2*i], argv[1][2*i+1], 0 };
		if ( sscanf( tmp, "%hhx", &(key[i]) ) == 1 ) continue;
		printf("error in hex string\n"); exit(2);
	}

	// get start offset
	int crc = dl_crypt_crc12(key,16);
	int offset = dl_crypt_ofsbuffer[crc];
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
			for (int j = 0; j < size; j++) buffer[j] = buffer[j] ^ dl_crypt_keybuffer[j+offset];
			offset += size;
			offset %= 4095;
			fwrite( buffer, 1, size, out );
		}

		fclose(in);
		fclose(out);
	}
}

