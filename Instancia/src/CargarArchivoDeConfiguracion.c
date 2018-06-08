/*
 * cargarArchivoDeConfig.c
 *
 *  Created on: 7 jun. 2018
 *      Author: utnso
 */

#include <commons/config.h>

//Configuracion
char * IP_COORDINADOR;
int PUERTO_COORDINADOR;
char * PUNTO_DE_MONTAJE;
char * NOMBRE_INSTANCIA;
int INTERVALO_DUMP; // Debe ser int

//Nombres para el archivo de configuracion
#define ARCH_CONFIG_IP_COORD "IP COORDINADOR"
#define ARCH_CONFIG_PUERTO_COORD "PUERTO COORDINADOR"
#define ARCH_CONFIG_PUNTO_MONTAJE "PUNTO DE MONTAJE"
#define ARCH_CONFIG_NOMBRE_INSTANCIA "NOMBRE INSTANCIA"
#define ARCH_CONFIG_INTERVALO_DUMP "INTERVALO DUMP"

void cargarArchivoDeConfig(char *path) {
	if (path != NULL) {

		printf("Cargando archivo de configuracion...\n");
		t_config * config_file = config_create(path);

		if (config_has_property(config_file,ARCH_CONFIG_IP_COORD)){
			IP_COORDINADOR = strdup(config_get_string_value(config_file, ARCH_CONFIG_IP_COORD));
			printf("\tIP Coordinador: %s\n", IP_COORDINADOR);
		}
		if (config_has_property(config_file,ARCH_CONFIG_PUERTO_COORD)){
			PUERTO_COORDINADOR = config_get_int_value(config_file, ARCH_CONFIG_PUERTO_COORD);
			printf("\tPuerto Coordinador: %d\n", PUERTO_COORDINADOR);
		}

		if (config_has_property(config_file,ARCH_CONFIG_PUNTO_MONTAJE)){
			PUNTO_DE_MONTAJE = strdup(config_get_string_value(config_file, ARCH_CONFIG_PUNTO_MONTAJE));
			printf("\tPunto de montaje: %s\n", PUNTO_DE_MONTAJE);
		}

		if (config_has_property(config_file,ARCH_CONFIG_NOMBRE_INSTANCIA)){
			NOMBRE_INSTANCIA = strdup(config_get_string_value(config_file, ARCH_CONFIG_NOMBRE_INSTANCIA));
			printf("\tNombre de Instancia: %s\n", NOMBRE_INSTANCIA);
		}

		if (config_has_property(config_file,ARCH_CONFIG_INTERVALO_DUMP)){
			INTERVALO_DUMP = config_get_int_value(config_file, ARCH_CONFIG_INTERVALO_DUMP);
			printf("\tIntervalo dump: %d\n", INTERVALO_DUMP);
		}

		config_destroy(config_file);
	}
	else {
		printf("Error al cargar el archivo de configuracion...\n");
		exit(1);
	}
}
