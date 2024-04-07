#include <stdio.h>
#include <stdlib.h>

// Definimos la estructura de las tablas de particion, cada una de 16 bytes
typedef struct 
{
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned short starting_cluster;
    unsigned int file_size;
} __attribute((packed)) PartitionTable;

// Definimos la estructura del Boot Sector. de tamaño 512 bytes
typedef struct 
{
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;

    unsigned char sectors_per_cluster; 			// 1 byte
    unsigned short number_of_reserved_sectors; 	// 2 bytes
    unsigned char number_of_fats; 				// 1 byte
	unsigned short root_dir_entries;			// 2 bytes
	unsigned short num_sectors; 				// 2 bytes.
    unsigned char media_type; 					// 1 byte. (0xf0 = disco extraíble, 0xf8 = disco fijo).
    unsigned short size_fat_in_sectors; 		// 2 bytes
    unsigned short sectors_per_track; 			// 2 bytes
    unsigned short num_headers; 				// 2 bytes
    unsigned int count_of_hidden_sectors; 		// 4 bytes 
    unsigned int total_logical_sectors; 		// 4 bytes.
    unsigned char num_physical_drive; 			// 1 byte.
    unsigned char ignore; 						// 1 byte
    unsigned char boot_signature; // 1 byte. Firma de arranque extendida

    char volume_id[4];
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;


// Definimos la estructura del Directory entry. 32 bytes cada uno.
typedef struct 
{
    unsigned char filename[1]; //Primer carácter del nombre de archivo o el estado de asignación (0x00 sin asignar, 0xE5 eliminado) 
    unsigned char name[7];
    unsigned char extension[3];
    unsigned char attributes[1]; 			//0x10 = Directorio  // 0x20 = Archivo.
    unsigned char reserved[1];
    unsigned char created_time_in_seconds[1];
    unsigned short created_time_in_hours_minutes_seconds; 	
    unsigned short created_day; 							
    unsigned short accessed_day; 							
    unsigned short high_first_cluster_address; 				
    unsigned short written_time_in_hours_minutes_seconds; 	
    unsigned short written_day; 							
    unsigned short low_first_cluster_address; 				
    unsigned int size_of_file; 								

} __attribute((packed)) Fat12Entry;

void recover_file(unsigned short pointer)
{
    FILE * in = fopen("test.img", "r+");

    char charAux[1] ="L";
    fseek(in, pointer , SEEK_SET);
    fwrite(charAux, sizeof(char), sizeof(charAux), in);
    printf("Archivo se ha restaurado correctamente!\n");
            
    fclose(in);
}

void print_file_info(Fat12Entry *entry, unsigned short pointer_recover_file)
{
    switch(entry->filename[0])
    {
        case 0x00:
            return;
        case 0xE5:
            printf("Archivo borrado: [%c%.7s.%.3s]\n", 0xE5, entry->name, entry->extension);
            recover_file(pointer_recover_file);    
            return;
        default:
            return;
    }
}

int main() 
{
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;

    unsigned short pointer_recover_file;

    fseek(in, 0x1BE , SEEK_SET);
    fread(pt, sizeof(PartitionTable), 4, in);

    printf("#####################################################\n");
    
    for(i=0; i<4; i++) 
    {        
        if(pt[i].partition_type == 1) 
        {
            printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) 
    {
        printf("Exception: No se ha encontrado el filesystem FAT12\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);

    fread(&bs, sizeof(Fat12BootSector), 1, in);

    printf("#####################################################\n");

    fseek(in, (bs.number_of_reserved_sectors-1 + bs.size_fat_in_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);
    
    for(i=0; i<bs.root_dir_entries; i++) 
    {
        pointer_recover_file = ftell(in);
        
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry, pointer_recover_file);
    }
    
    fclose(in);
    return 0;
}

