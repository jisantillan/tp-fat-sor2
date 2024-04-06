# Trabajo Práctico - FAT o un tp muy grande
# Materia: Sistemas Operativos y Redes 2. 

En este trabajo, nos centraremos en explorar en profundidad el sistema de archivos FAT12, una de las versiones más antiguas del sistema de archivos FAT.
--- 
## Módulos
Para llevar a cabo este trabajo práctico hemos desarrollado los siguientes módulos:
  - read_boot.c : Este módulo se encarga de leer y analizar el sector de arranque del sistema de archivos FAT12.
  - read_mbr.c : Aquí, nos enfocamos en la lectura y análisis del MBR para obtener información sobre la estructura del disco.
  - read_root.c : El objetivo de este módulo es explorar el contenido del directory root en el sistema de archivos FAT12.
  - read_file.c : En este módulo, nos centramos en la lectura de archivos 
  - file_recovery.c : En este módulo, nos centramos en la recuperación de archivos 


## Como ejecutar código fuente

## Requisitos

    - Archivo <nombre_modulo>.c
    - Archivo Makefile
    - Estar situado en un directorio que contenga ambos archivos.
    

### Paso 1: Compilación del módulo

```bash
$ make <nombre_modulo>.c
```

### Paso 2: Ejecutar 

```bash
$ ./<nombre_modulo>
```

## Compilación en un paso de todos los modulos.
```bash
$ make all
```
## Limpiar directorio de trabajo
```bash
$ make clean
```

