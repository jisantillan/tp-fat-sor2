#include <stdio.h>
#include <stdlib.h>

static int partition_table_start = 446;

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    char length_sectors[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size; // 2 bytes
	// {...}  COMPLETAR
    unsigned char sectors_per_cluster;
    unsigned short reserved_area_size;
    unsigned char num_of_fat;
    unsigned short max_num_of_files_in_root;
    unsigned short num_of_sectors_in_fs;
    char media_type;
    unsigned short fat_size;
    unsigned short sectors_per_track;
    unsigned short heads;
    unsigned int num_of_sectors_before_start_partition;
    unsigned int fs_sectors;
    char drive_number;
    char not_used; //xd
    char extended_boot_signature;
    //
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8]; // Type en ascii
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    
    
    //lo hardcodee, ver forma de obtener el 446
    fseek(in, partition_table_start, SEEK_SET); // Ir al inicio de la tabla de particiones. Completar ...
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 
    
    for(i=0; i<4; i++) {        
        printf("Partiion type: %d\n", pt[i].partition_type);
        if(pt[i].partition_type == 1) {
            printf("Encontrado FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No se encontró filesystem FAT12, saliendo ...\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("  Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("  OEM code: [%.8s]\n", bs.oem);
    printf("  sector_size: %d\n", bs.sector_size);
	// {...} COMPLETAR
    printf("  Sectors per cluster: %d\n", bs.sectors_per_cluster);
    printf("  Reserved area size, in sectors: %d\n", bs.reserved_area_size);
    printf("  Number of FATs: %d\n", bs.num_of_fat);
    printf("  Max number of files in the root directory: %d\n", bs.max_num_of_files_in_root);
    printf("  Number of sectors in the file system: %d\n", bs.num_of_sectors_in_fs);
    printf("  Media type: %02X\n", bs.media_type);
    printf("  Size of each FAT, in sectors: %d\n", bs.fat_size);
    printf("  Number per track in storage device: %d\n", bs.sectors_per_track);
    printf("  Number of heads in storage device: %d\n", bs.heads);
    printf("  Number of sectors before the start partition: %d\n", bs.num_of_sectors_before_start_partition);
    printf("  Number of sectors in the file system: %d\n", bs.fs_sectors);
    printf("  BIOS INT 13h drivce number: %02X\n", bs.drive_number);
    printf("  Byte not used: %02X\n", bs.not_used);
    printf("  Extended boot signature: %02X\n", bs.extended_boot_signature);
    //
    printf("  volume_id: 0x%08X\n", bs.volume_id);
    printf("  Volume label: [%.11s]\n", bs.volume_label);
    printf("  Filesystem type: [%.8s]\n", bs.fs_type);
    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);
    
    fclose(in);
    return 0;
}
