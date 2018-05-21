/*
 * redis_lib.h
 *
 *  Created on: 20 may. 2018
 *      Author: utnso
 */

#ifndef REDIS_LIB_H_
#define REDIS_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// Para manejo de strings
#include <sys/socket.h>		// Para crear los sockets
#include <netdb.h> 			// Para getaddrinfo
#include <unistd.h> 		// Para close(socket)

/**********************************************/
/* PROTOCOLO								  */
/**********************************************/
struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

enum procesos { esi, instancia, planificador, coordinador };

/**********************************************/
/* FUNCIONES								  */
/**********************************************/

/**
* @NAME: crear_listen_socket
* @DESC: Crea un socket escuchando un puerto y lo devuelve. Devuelve -1 si falla la conexion
* @PARAM:	puerto			->	Puerto a escuchar
* 			max_conexiones	->	Maximo de conexiones para listen
*/
int crear_listen_socket(char* puerto, int max_conexiones);

/**
* @NAME: conectar_a_server
* @DESC: Crea un nuevo socket, lo conecta a una ip puerto y lo devuelve. Devuelve -1 si falla la conexion
* @PARAM:	ip				-> 	IP a conectar
* 			puerto			->	Puerto a conectar
*/
int conectar_a_server(char* ip, char* puerto);













#endif /* REDIS_LIB_H_ */
