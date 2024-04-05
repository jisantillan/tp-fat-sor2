#include <stdio.h>
#include <stdlib.h>

static int partition_table_start = 446;
static int contador_archivos = 0;

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    // char starting_cluster[4];
    // char file_size[4];
    unsigned int starting_cluster;
    unsigned int file_size;
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
	// {...} COMPLETAR
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
    char not_used; //xd
    char extended_boot_signature;
    //
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
	// {...} COMPLETAR
    // char first_char_filename;
    unsigned char filename[8];
    char extension[3];
    char attributes;
    char reserved;
    char file_creation_time; //en decimas de segundos
    short creation_time; //horas, minutos, segundos
    short creation_date;
    short last_access_date;
    unsigned short first_cluster_number_MSB;
    short last_mod_time;
    short last_mod_date;
    unsigned short first_cluster_number_LSB;
    unsigned int file_size;
} __attribute((packed)) Fat12Entry;

/* Devuelve 1 si es valido, 0 si es invalido
TODO ver si este criterio mas el de is_valid_filename es suficiente para identificar archivos/directorios */
int is_valid_filename_character(unsigned char *c){
    int ret = 1;
    char padding = 0x20;     //caracter que representa el espacio con el que se rellena el filename/extension del archivo/directorio
    char first_lowercase_letter = 0x61;
    char last_uppercase_letter = 0x7A;
    
    //si no es paddin y esta dentro del rango de letras minusculas... mejorar la logica para que se lea mejor
    if(*c != padding && (*c >= first_lowercase_letter && *c <= last_uppercase_letter)){
        ret = 0;
    }
    return ret;
}

/* Devuelve 1 si es valido, 0 si es invalido
    Sabemos que una entrada del directory entry, dentro de filename y extension no puede terminar en 0x00 o null, 
    entonces usamos eso para filtrar los que no sean archivos/directorios, ademas de que tienen que estar todos en uppercase */
int is_valid_filename(Fat12Entry *entry){
    
    // printf("ANALIZANDO: [%.8s.%.3s]\n", entry->filename, entry->extension);
    int ret = 1;
    if(entry->extension[2] == 0x00){
        ret = 0;
    }
    else{
        int i;
        for(i = 0; i < 8; i++){ //Máximo 8 carateres para los nombres
            if(!is_valid_filename_character(&entry->filename[i])){
                ret = 0;
                break;
            }
        }
        for(i = 0; i < 3; i++){ //Máximo 3 para extensiones
            if(!is_valid_filename_character(&entry->extension[i])){
                 ret = 0;
                  break;
            }
        }
    }
    return ret;
}

void print_file(Fat12Entry *entry){
    printf("  File Size[%d]\n", entry->file_size);
    printf("  First cluster number LSB: %X - 0x%02X\n", entry->first_cluster_number_LSB, entry->first_cluster_number_LSB);
}

void print_file_info(Fat12Entry *entry) {
    switch(entry->filename[0]) {
    case 0x00: //Entrada sin usar
        return; 
    case 0xE5: //Entrada borrada (tanto directorio como archivo)
        if(is_valid_filename(entry)){
            printf("Archivo borrado: [?%.7s.%.3s]\n", entry->filename + 1, entry->extension);
            // print_file(entry);
        }
        return;
    case 0x05: //La entrada libre para uso
        if(is_valid_filename(entry)){
            printf("Archivo que comienza con 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename + 1, entry->extension);
            // print_file(entry);
        }
        break;
    case 0x2E: //Entrada es un directorio
        if(is_valid_filename(entry)){
            printf("Directorio: [%.8s.%.3s]\n", entry->filename, entry->extension);
            // printf("  Segundo byte: %02X\n", entry->filename[1]);
            // print_file(entry);
        }
        break;
    default:
        // Aca se verifica si la entrada es un directorio o archivo 
        if(is_valid_filename(entry)){
	    if (entry->attributes == 32) { // 32 es 20 es hexa
                printf("Archivo: [%.8s.%.3s]\n", entry->filename, entry->extension);
		contador_archivos++;
	    }
	    if (entry->attributes == 16) { // 16 es 10 en hexa
	         printf("Directorio: [%.8s.%.3s]\n", entry->filename, entry->extension);
	    }
             //print_file(entry);
        }
        return;
    }
}

/* Funcion para ver algunos datos de la particion, copiado de read_mbr */
void print_partition(PartitionTable *partition){
        printf("  Partition entry\n  First byte %02X\n", partition->first_byte);
        printf("  Comienzo de partición en CHS: %02X:%02X:%02X\n", partition->start_chs[0], partition->start_chs[1], partition->start_chs[2]);
        printf("  Partition type 0x%02X\n", partition->partition_type);
        printf("  Fin de partición en CHS: %02X:%02X:%02X\n", partition->end_chs[0], partition->end_chs[1], partition->end_chs[2]);
        printf("  Dirección LBA relativa (starting cluster) 0x%08X\n", partition->starting_cluster);
        printf("  Tamaño en sectores %d\n\n", partition->file_size);
}

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;

	//{...} Completar
    fseek(in, partition_table_start, SEEK_SET); // Ir al inicio de la tabla de particiones. Completar ...
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas

    for(i=0; i<4; i++) {
        if(pt[i].partition_type == 1) {
            printf("Encontrada particion FAT12 %d\n", i);
            print_partition(&pt[i]);
            printf("\n");
            break;
        }
    }

    if(i == 4) {
        printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET);
	//{...} Leo boot sector
    fread(&bs, sizeof(Fat12BootSector), 1, in);//Completar luego...

    printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs, position to start reading: %d\n\n", 
           ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats, ((bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) * bs.sector_size));

    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_CUR);

    printf("Root dir_entries %d \n", bs.root_dir_entries);
    
    for(i=0; i<bs.root_dir_entries; i++) {
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }

    printf("\nLeido Root directory, ahora en 0x%X\n", ftell(in));

    int j;
    int bytes_read = ftell(in); //los bytes que ya lei
    int total_bytes = (bs.num_of_sectors_in_fs * bs.sector_size); //la cantidad total de bytes
    int bytes_left = total_bytes - bytes_read; //los bytes que me faltan leer
    int entries_left = bytes_left / 32; //las entradas que me faltan leer. Divido por 32 ya que una directory entry es de 32 bytes
    printf("Entradas faltantes: %d\n", entries_left);
    for(j=0; j < entries_left; j++){
        fread(&entry, sizeof(entry), 1, in);
        print_file_info(&entry);
    }
    printf("Se encontraron %d archivos. \n", contador_archivos);
    fclose(in);
    return 0;
}
