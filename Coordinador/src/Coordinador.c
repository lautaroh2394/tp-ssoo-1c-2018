/*
 ============================================================================
 Name        : Coordinador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */
#include "Utilidades.h"
#include "FuncionesCoordinador.c"

int enviarSentenciaInstancia(t_sentencia * sentencia){
	switch(sentencia->keyword){
	/*case set:
		break;*/
	case STORE:
		break;
	default:
		//set
		//asignar instancia
		break;
	}

	t_instancia * proxima = siguienteInstanciaSegunAlgoritmo();
	log_info(logger,"Enviando a instancia: '%s' %s %s",proxima->nombre,sentencia->clave, sentencia->valor);

	if (string_equals_ignore_case(proxima->nombre, "ERROR")){
		free(proxima->nombre);
		free(proxima);

		return;
	}

	t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_SENTENCIA, sizeof(t_content_header));
	t_esi_operacion_sin_puntero * s_sin_p = armar_esi_operacion_sin_puntero(sentencia);
	int header_envio = send(proxima->socket,header,sizeof(t_content_header),NULL);
	int sentencia_envio = send(proxima->socket, s_sin_p, sizeof(t_esi_operacion_sin_puntero),NULL);
	if (sentencia->keyword == SET){
		int valor_envio = send(proxima->socket,sentencia->valor,strlen(sentencia->valor),NULL);
	}
	log_info(logger,"Enviada sentencia a instancia");

	log_info(logger, "Esperando rta de Instancia");

	int header_rta_instancia = recv(proxima->socket,header,sizeof(t_content_header), NULL);
	log_info(logger, "Rta Instancia Header: - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",header->proceso_origen,header->proceso_receptor,header->operacion,header->cantidad_a_leer);

//TODO: que enviarSentenciaInstancia devuelva el codigo de rdo a devolver al esi.
	free(proxima->nombre);
	free(proxima);
	free(header);
	free(s_sin_p);

	return 1;
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	log_info(logger,"%d",debug_var); debug_var++;
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			//TODO leer packete para obtener nombre.
			char * nombre = malloc(hd->cantidad_a_leer);
			int status_recv = recv(socketInstancia, nombre, hd->cantidad_a_leer, NULL);
			log_info(logger,"Tamaño nombre: %d - Nombre: %s",sizeof(nombre),nombre);
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia, nombre);

			free(nombre);
			break;
		default:
			break;
	}
}

void interpretarOperacionPlanificador(t_content_header * hd, int socketCliente){
	log_info(logger,"Interpetando operación planificador");
	switch(hd->operacion){
	case PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION:
		PROCESO_PLANIFICADOR.id = nuevoIDInstancia();
		PROCESO_PLANIFICADOR.socket = socketCliente;
	}
}

int puedoEjecutarSentencia(t_sentencia * sentencia){

	//TODO Verificar existencia de clave en alguna instancia.
	//	   Si no existe, crearla (internamente?)
	//	   Si existe, verificar conexión de instancia.
	//	   Si no está conectada abortar esi.
	//TODO Preguntarle a Planificador si la clave no esta bloqueada.
	log_info(logger,"Puedo ejecutar?");
	if(	hay_instancias != 0){return CORRECTO;}else return ABORTAR;
}

void devolverResultadoInstanciaAESI(int socketCliente, int cod){

	t_content_header * cabecera_error = crear_cabecera_mensaje(coordinador,esi, RESULTADO_EJECUCION_SENTENCIA,sizeof(t_content_header));
	int status_hd_error = send(socketCliente,cabecera_error,sizeof(t_content_header),NULL);

	respuesta_coordinador * cod_error = malloc(sizeof(respuesta_coordinador));
	cod_error->resultado_del_parseado = cod;
	int status_hd_mensaje_error = send(socketCliente, cod_error, sizeof(respuesta_coordinador), NULL);
}

void indicarCompactacionATodasLasInstancias(){
	//TODO
}


void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando operación ESI - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
		;
		//Recibo de ESI sentencia parseada
		log_info(logger,"esi sentencia recibo");
		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		int cantleida = recv( socketCliente, sentencia, hd->cantidad_a_leer, NULL);
		log_info(logger,"esi sentencia recibida");
		log_info(logger,"KEYWORD: %d", sentencia->keyword);
		log_info(logger,"tam valor: %d", sentencia->tam_valor);

		char * valor;
		if (sentencia->keyword == SET){
			//Recibo el valor - El esi me lo manda "pelado", directamente el string, ningún struct
			valor = malloc(sentencia->tam_valor);
			int valor_status = recv(socketCliente, valor, sentencia->tam_valor,NULL);
			//valor[strlen(valor)-1] = '\0';
			log_info(logger,"esi valor recibido: %s", valor);
		}else valor = "";

		//Armo una variable interna para manejar la sentencia
		t_sentencia * sentencia_con_punteros = armar_sentencia(sentencia, valor);
		log_operacion_esi(sentencia_con_punteros, logger_operaciones);

		int puedoEnviar = puedoEjecutarSentencia(sentencia_con_punteros);
		log_info(logger, "puedo ejecutar? %d", puedoEnviar);
		switch(puedoEnviar){
			case CORRECTO:
				;
				int rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);
				while (rdo_ejecucion_instancia == COMPACTACION){
					indicarCompactacionATodasLasInstancias();
					rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);
				}
				devolverResultadoInstanciaAESI(socketCliente, CLAVE_BLOQUEADA);
				break;
			case CLAVE_BLOQUEADA:
				log_info(logger,"Devolviendo error al ESI%d", sentencia->pid);
				devolverResultadoInstanciaAESI(socketCliente, CLAVE_BLOQUEADA);
				log_info(logger,"Devuelto CLAVE_BLOQUEADA");
				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
				break;
			case ABORTAR:
				log_info(logger,"Devolviendo error al ESI %d", sentencia->pid);
				devolverResultadoInstanciaAESI(socketCliente, ABORTAR);
				log_warning(logger,"Devuelto ABORTAR al ESI %d", sentencia->pid);
				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
				break;
			default:
				break;
		}
		free(sentencia);
		break;
	default:
		//TODO no se reconoció el tipo operación
		break;
	}
	log_info(logger,"fin interpret");
}

void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case esi:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case instancia:
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case planificador:
		interpretarOperacionPlanificador(hd,socketCliente);
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    log_info(logger, "Cliente conectado. Esperando mensajes:");
    total_hilos++;
    log_info(logger, "total hilos: %d",total_hilos);

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header != 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	log_info(logger, "Recv - Status Header: %d", status_header);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado"); total_hilos--;
    	}
    	else {
    		log_info(logger, "Interpretando header...");
    		interpretarHeader(header, socketCliente);
    	};
   	}
    free(header);
    close(socketCliente);
}

int main(int argc, char **argv){

	seteosIniciales(argv[1]);

	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(logger,"Socket de escucha creado %d", listenningSocket);

    // Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    log_info(logger, "Socket de escucha bindeado");
    freeaddrinfo(serverInfo);

    log_info(logger, "Escuchando...");
    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

	while (1){
    	log_info(logger, "Esperando conexiones...");
    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
		log_info(logger, "Conexión recibida - Accept: %d ",socketCliente);

		crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);
	}

    close(listenningSocket);
    log_destroy(logger);
    return 0;


}
