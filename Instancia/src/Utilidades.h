/*
 * Utilidades.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef SRC_UTILIDADES_H_
#define SRC_UTILIDADES_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/collections/list.h> // Para manejo de strings

#define NOMBRE_INSTANCIA "Instancia1"
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR 8080
#define PACKAGESIZE 1024
#define INTERVALO_DUMP 10 // Intervalo dado en segundos para guardar la tabla de entradas en archivo de texto plano
#define PUNTO_DE_MONTAJE "home/utnso/instanciaX"

#define GET_KEYWORD 0
#define SET_KEYWORD 1

//***Cod Procesos
#define ESI 1
#define INSTANCIA 2
#define PLANIFICADOR 3
#define COORDINADOR 4

//***

//***Cod ops
#define INSTANCIA_COORDINADOR_CONEXION 1
#define COORDINADOR_INSTANCIA_CONFIG_INICIAL 2
#define COORDINADOR_INSTANCIA_SENTENCIA 3
#define INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA 4

//***

struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

// struct para el envio de nombre de Instancia al Coordinador
typedef struct{
	char nombreInstancia[40];
} __attribute__((packed)) t_info_instancia;

typedef struct{
	int keyword;
	char clave[40];
	int tam_valor;
	int pid;
	} __attribute__((packed)) t_esi_operacion_sin_puntero;

typedef struct{
		int keyword;
		char clave[40];
		char * valor;
	} t_sentencia;

// Lista de Entradas

t_list * l_entradas;

// Struct para la configuracion inicial de Tabla de Entradas

typedef struct {
	int cantTotalEntradas;
	int tamanioEntradas;
} __attribute__((packed)) t_config_tabla_entradas;

t_config_tabla_entradas * configTablaEntradas;

// Estructura de Entradas
typedef struct{
	char clave[40];
	int numeroEntrada;
	int tamanioEntrada;
} __attribute__((packed)) t_entrada;


#endif /* SRC_UTILIDADES_H_ */
