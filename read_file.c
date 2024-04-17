#include <stdio.h>
#include <stdlib.h>

static int partition_table_start = 446;

typedef struct
{
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    // char starting_cluster[4];
    // char file_size[4];
    unsigned int starting_cluster;
    unsigned int file_size;
} __attribute((packed)) PartitionTable;

typedef struct
{
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short num_of_sectors_in_fs;
    char media_type;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short heads;
    unsigned int num_of_sectors_before_start_partition;
    unsigned int fs_sectors;
    char drive_number;
    char not_used;
    char extended_boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct
{
    // char first_char_filename;
    unsigned char filename[8];
    char extension[3];
    char attributes;
    char reserved;
    char file_creation_time; // en decimas de segundos
    short creation_time;     // horas, minutos, segundos
    short creation_date;
    short last_access_date;
    unsigned short first_cluster_number_MSB;
    short last_mod_time;
    short last_mod_date;
    unsigned short first_cluster_number_LSB;
    unsigned int file_size;
} __attribute((packed)) Fat12Entry;

/* Devuelve 1 si es archivo o directorio, 0 si no
    para obtener este dato, se verifica que el dato del atributo indique una de estas dos posibilidades.
    Para archivos, el valor de este byte sera 32 (0x20); y para directorios sera 16 (0x10)*/
int is_file_or_dir(Fat12Entry *entry){
    return entry->attributes == 32 || entry->attributes == 16;
}

void read_and_print_file(unsigned short file_position, unsigned int file_size)
{
    FILE *fs = fopen("test.img", "rb");
    char file_content[file_size];
    fseek(fs, file_position, SEEK_SET);    // nos movemos a la posicion del archivo
    fread(file_content, file_size, 1, fs); // leemos bytes de acuerdo al tamaño del archivo desde donde habiamos posicionado el puntero, y lo guardamos en file_content
    printf("\n");
    for (int i = 0; i < file_size; i++)
    {
        printf("%c", file_content[i]); // imprimimos el contenido
    }
    printf("\n");
    fclose(fs);
}

unsigned int get_file_position(Fat12Entry *entry, unsigned short first_cluster, unsigned short cluster_size)
{
    unsigned short first_cluster_number_lsb = entry->first_cluster_number_LSB;                    // The First Logical Cluster field specifies where the file or subdirectory begins. Note that it gives the value of the FAT index.
    unsigned int file_position = first_cluster + ((first_cluster_number_lsb - 2) * cluster_size); // Restamos 2 por las primeras dos entradas que son reservadas.
    return file_position;
}

void print_file_info(Fat12Entry *entry, unsigned short first_cluster, unsigned short cluster_size)
{
    switch (entry->filename[0])
    {
    case 0x00:
        return; // unused entry
    case 0xE5:  // borrado
        return;
    case 0x05: // libre para uso
        break;
    case 0x2E: // directorio
        break;
    default:
        switch (entry->attributes)
        {
        case 0x0f: // longfile
            break;
        case 0x10: // subdir
            break;
        case 0x20:
            if (is_file_or_dir(entry))
            {
                printf("Archivo: [%.8s.%.3s]", entry->filename, entry->extension);
                read_and_print_file(get_file_position(entry, first_cluster, cluster_size), entry->file_size);
                break;
            }
        default:
            break;
        }
        break;
    }
}

int main()
{
    FILE *in = fopen("test.img", "rb");
    if (in == NULL)
    {
        printf("Error al abrir el archivo");
        return -1;
    }
    
    int i;
    unsigned short root_directory_position;
    unsigned short first_cluster;
    unsigned short size_cluster;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;

   
    fseek(in, partition_table_start, SEEK_SET); // Ir al inicio de la tabla de particiones. 
    fread(pt, sizeof(PartitionTable), 4, in);   // leo entradas

    for (i = 0; i < 4; i++)
    {
        if (pt[i].partition_type == 1)
        {
            printf("Encontrada particion FAT12 %d\n", i);
            printf("\n");
            break;
        }
    }

    if (i == 4)
    {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET);
    // Leo boot sector
    fread(&bs, sizeof(Fat12BootSector), 1, in);

    printf("En  0x%lX, sector size %d, FAT size %d sectors, %d FATs, position to start reading: %d\n\n",
    ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats, ((bs.reserved_sectors - 1 + bs.fat_size_sectors * bs.number_of_fats) * bs.sector_size));

    // nos posicionamos en el root directory --> // (1-1 + 2 * 2) * 512(Ax200) = 2560 (0xA00)
    fseek(in, (bs.reserved_sectors - 1 + bs.fat_size_sectors * bs.number_of_fats) * bs.sector_size, SEEK_CUR);

    root_directory_position = ftell(in);                                             // posición de root directory, calculado previamente
    first_cluster = root_directory_position + (bs.root_dir_entries * sizeof(entry)); // posicion del primer cluster
    size_cluster = bs.sectors_per_cluster * bs.sector_size;                          // tamaño del cluster

    printf("Root dir_entries %d \n\n", bs.root_dir_entries);
    for (i = 0; i < bs.root_dir_entries; i++)
    {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry, first_cluster, size_cluster);
    }

    printf("Leido Root directory, ahora en 0x%lX\n", ftell(in));
    fclose(in);
    return 0;
}
