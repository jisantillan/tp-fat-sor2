#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE * in = fopen("test.img", "rb");
    unsigned int i, start_sector, length_sectors;

    fseek(in, 000, SEEK_SET);

   puts("Bootstrap code area:");
    for (i=0; i<446;i++){
	printf("%02X ", fgetc(in));
/*	printf("byte numero: %d\n", i);*/
   }
    puts("\nPartition entries:");

    for(i=0; i<4; i++) {
        printf("Partition entry %d: First byte %02X\n", i, fgetc(in));
        printf("  Comienzo de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
        printf("  Partition type 0x%02X\n", fgetc(in));
        printf("  Fin de partición en CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));

        fread(&start_sector, 4, 1, in);
        fread(&length_sectors, 4, 1, in);
        printf("  Dirección LBA relativa 0x%08X, de tamaño en sectores %d\n", start_sector, length_sectors);
    }

    puts("Boot signature:  ");
    for(i=0; i<2;i++){
	printf("%02X ", fgetc(in));
    }
    puts("");

    fclose(in);
    return 0;
}
