/*
 ============================================================================
 Name        : Planificador.c
 Author      : La Orden Del Socket
 Version     : 0.1
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Planificador
 ============================================================================
 */

#include "Planificador.h"

int main(void) {

	fd_set readset, writeset, exepset;
	int max_fd;
	char read_buffer[MAX_LINEA];

	//Creo el socket servidor para recibir ESIs (ya bindeado y escuchando)
	int serv_socket = iniciar_servidor(PORT_ESCUCHA);

	//Creo el socket cliente para conectarse al coordinador
	int coord_socket = conectar_coordinador(IP_COORD, PORT_COORD );

	crear_listas_planificador();
	inicializar_conexiones_esi();
	stdin_no_bloqueante();

	while(TRUE){
		//Inicializa los file descriptor
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_ZERO(&exepset);

		//Agrega el fd del socket servidor al set de lectura y excepciones
		FD_SET(serv_socket, &readset);
		FD_SET(serv_socket, &exepset);

		//Agrega el fd del socket coordinador al set de lectura
		FD_SET(coord_socket, &readset);
		//FD_SET(coord_socket, &writeset);
		FD_SET(coord_socket, &exepset);

		//Agrega el stdin para leer la consola
		FD_SET(STDIN_FILENO, &readset);
		//FD_SET(fileno(stdin), &exepset);

		/* Seteo el maximo file descriptor necesario para el select */
		max_fd = serv_socket;

		//Agrega los conexiones esi existentes
		for (int i = 0; i < MAX_CLIENTES; i++)
		{
			if (conexiones_esi[i].socket != NO_SOCKET)
			{
				FD_SET(conexiones_esi[i].socket, &readset);
				FD_SET(conexiones_esi[i].socket, &writeset);
				FD_SET(conexiones_esi[i].socket, &exepset);
			}

			if (conexiones_esi[i].socket > max_fd)
				max_fd = conexiones_esi[i].socket;

		}

		if(max_fd < coord_socket)
			max_fd = coord_socket;

		int result = select(max_fd+1, &readset, &writeset, &exepset, NULL); //&time);

		if(result == 0)
			printf("Select time out\n");
		else if(result < 0){
			printf("Error en select\n");
			exit(EXIT_FAILURE);
		}

		else if(result > 0) //Hubo un cambio en algun fd
		{

			//Aceptar nuevas conexiones de ESI
			if (FD_ISSET(serv_socket, &readset)) {
				int nuevo_esi = atender_nuevo_esi(serv_socket);

			}

			//Atender al coordinador
			if(FD_ISSET(coord_socket, &readset))
			{
				if(recibir_mensaje_coordinador(coord_socket) == 0)
				{
					cerrar_conexion_coord(coord_socket);
				}
			}


			/*if(FD_ISSET(coord_socket, &writeset))
			{
				printf("Entro al isset del coord WRITE\n");
				recibir_mensaje_coordinador(coord_socket);
			}
			 */
			if(FD_ISSET(coord_socket, &exepset))
			{
				if(recibir_mensaje_coordinador(coord_socket) == 0)
				{
					cerrar_conexion_coord(coord_socket);
				}
			}

			//Se ingresó algo a la consola
			if(FD_ISSET(STDIN_FILENO, &readset))
			{

				read_from_stdin(read_buffer, MAX_LINEA);

				int res = comando_consola(read_buffer);
				if(res)
					break;

			}

			//Manejo de conexiones esi ya existentes
			for (int i = 0; i < MAX_CLIENTES; ++i) {
				if (conexiones_esi[i].socket != NO_SOCKET ){
					//Mensajes nuevos de algun esi
					if (FD_ISSET(conexiones_esi[i].socket, &readset)) {
						if(recibir_mensaje_esi(conexiones_esi[i].socket) == 0)
						{
							cerrar_conexion_esi(&conexiones_esi[i]);
							continue;
						}

					//Excepciones del esi, para la desconexion
						if (FD_ISSET(conexiones_esi[i].socket, &exepset)) {
							if(recibir_mensaje_esi(conexiones_esi[i].socket) == 0)
							{
								cerrar_conexion_esi(&conexiones_esi[i]);
								continue;
							}
						}
					} //if isset
				} // if NO_SOCKET
			} //for conexiones_esi
		} //if result select
	} //while

	return EXIT_SUCCESS;
}

int conectar_coordinador(char * ip, char * port) {

	int familiaProt = PF_INET;
	int tipo_socket = SOCK_STREAM;
	int protocolo = 0; //ROTO_TCP;

	struct addrinfo hints;
	struct addrinfo *coord_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(ip, port, &hints, &coord_info); // Carga en server_info los datos de la conexion


	printf("\nIniciando como cliente hacia el coordinador...\n");

	int coord_socket = socket(coord_info->ai_family, coord_info->ai_socktype, coord_info->ai_protocol);
	if (coord_socket != -1)
		printf("Socket creado correctamente!\n");
	else
		printf("Error al crear el socket\n");

	int activado = 1;
	setsockopt(coord_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	//TODO probar accept en un servidor dummy
	int res_connect = connect(coord_socket, coord_info->ai_addr, coord_info->ai_addrlen);
	if (res_connect < 0)
	{
		printf("Error al intentar conectar al coordinador\n");
		close(coord_socket);
		exit(EXIT_FAILURE);
	}
	else
		printf("Conectado con el coordinador! (%d) \n",coord_socket);

	freeaddrinfo(coord_info);

	return coord_socket;


}


int iniciar_servidor(unsigned short port)
{

	struct sockaddr_in dir_serv_sock;

	unsigned int dir_cli_size;

	dir_serv_sock.sin_family = AF_INET;
	dir_serv_sock.sin_addr.s_addr = INADDR_ANY;
	dir_serv_sock.sin_port = htons(port);

	printf("Creando socket servidor...\n");

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));


	if(bind(server_socket, (void*)&dir_serv_sock, sizeof(dir_serv_sock)) != 0)
	{
		printf("Falló el bind del socket servidor\n");
		exit(1);
	}


	printf("Socket servidor (%d) escuchando\n", server_socket);
	listen(server_socket, MAX_CLIENTES);

	return server_socket;

	/*

	struct sockaddr_in dir_cli_pl;

	int cliente = accept(serv_pl, (void*)&dir_cli_pl, &dir_cli_size);
	printf("Recibí una nueva coneccion (%d) \n", cliente);
	send(cliente, "Te conectaste al planificador!\n",32,0);

	//for(;;);


	close(serv_pl);
	pthread_exit(0);
*/

}

/*	Consola preparada para funcionar en un hilo aparte
 * 	actualmente no se está usando ya que readline es una funcion bloqueante
 * 	y necesito que el select pueda leer de stdin mientras maneja las conexiones
 * 	Es código duplicado, pero la dejo porque está bonita :)
 */

void *consola() {

	int res=0;
	char *buffer = NULL;

	printf("\nAbriendo consola...\n");

	while(TRUE){

		//Trae la linea de consola
		buffer = readline(">");

		res = comando_consola(buffer);

		free(buffer);

		//Sale de la consola con exit
		if(res)
			break;

	}

	pthread_exit(0);

}

int obtener_key_comando(char* comando)
{
	int key = -1;

	if(comando == NULL)
		return key;

	if(!strcmp(comando, "pausar"))
		key = pausar;

	if(!strcmp(comando, "continuar"))
		key = continuar;

	if(!strcmp(comando, "bloquear"))
		key = bloquear;

	if(!strcmp(comando, "desbloquear"))
		key = desbloquear;

	if(!strcmp(comando, "listar"))
		key = listar;

	if(!strcmp(comando, "kill"))
		key = kill;

	if(!strcmp(comando, "status"))
		key = status;

	if(!strcmp(comando, "deadlock"))
		key = deadlock;

	if(!strcmp(comando, "exit"))
		key = salir;

	return key;
}

void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2)
{
	char** comandos;
	int i,j=0;

	comandos = string_n_split(buffer,3," ");

	while(comandos[j])
	{
		switch(j)
		{
			case 0:
				*comando = comandos[j];
				break;
			case 1:
				*parametro1 = comandos[j];
				break;
			case 2:
				*parametro2 = comandos[j];
				break;
		}

		j++;
	}


	for(i=0;i>j;i++)
	{
		printf("parte %d: %s\n", j,comandos[j]);
		free(comandos[j]);
	}
	free(comandos);

}

void pausar_consola(void)
{
	printf("Estas intentando pausar...\n");
	return;
}

void continuar_consola(void)
{
	printf("Estas intentando continuar...\n");
	return;
}

void bloquear_clave(char* clave , char* id)
{
	if(clave == NULL || id == NULL)
		printf("Parametros incorrectos (bloquear <clave> <id>)\n");
	else
		printf("Bloquear clave: %s id: %s\n",clave, id);
	return;
}

void desbloquear_clave(char* clave, char* id)
{
	if(clave == NULL || id == NULL)
		printf("Parametros incorrectos (desbloquear <clave> <id>)\n");
	else
		printf("Desbloquear clave: %s id: %s\n",clave, id);

	return;

}

void listar_recurso(char* recurso)
{
	if(recurso == NULL)
		printf("Parametros incorrectos (listar <recurso>)\n");
	else
		printf("Listar recurso: %s\n",recurso);

	return;
}


void kill_id(char* id)
{
	if(id == NULL)
		printf("Parametros incorrectos (kill <id>)\n");
	else
		printf("KILL ID: %s\n",id);

	return;
}


void status_clave(char* clave)
{
	if(clave == NULL)
		printf("Parametros incorrectos (status <clave>)\n");
	else
		printf("Status clave: %s \n",clave);

	return;
}


void deadlock_consola(void)
{
	printf("Si tan solo supiera que es...\n");
	return;
}

int comando_consola(char * buffer){

	int comando_key;
	char *comando = NULL;
	char *parametro1 = NULL;
	char *parametro2 = NULL;
	int res = 0;

	//printf("string recibido: %s\n",buffer);

	// Separa la linea de consola en comando y sus parametros
	obtener_parametros(buffer, &comando, &parametro1, &parametro2);
	//printf("comando: %s p1: %s p2: %s\n",comando,parametro1,parametro2);

	// Obtiene la clave del comando a ejecutar para el switch
	comando_key = obtener_key_comando(comando);

	switch(comando_key){
		case pausar:
			pausar_consola();
			break;
		case continuar:
			continuar_consola();
			break;
		case bloquear:
			bloquear_clave(parametro1,parametro2);
			break;
		case desbloquear:
			desbloquear_clave(parametro1, parametro2);
			break;
		case listar:
			listar_recurso(parametro1);
			break;
		case kill:
			kill_id(parametro1);
			break;
		case status:
			status_clave(parametro1);
			break;
		case deadlock:
			deadlock_consola();
			break;
		case salir:
			res = 1;
			printf("Saliendo de la consola\n");
			break;
		default:
			printf("No reconozco el comando vieja...\n");
			break;
	}

	//Limpio el parametro 1
	if(parametro1 != NULL)
	{
		free(parametro1);
		parametro1 = NULL;
	}

	//Limpio el parametro 2
	if(parametro2 != NULL)
	{
		free(parametro2);
		parametro2 = NULL;
	}


	free(comando);

	return res;

}

int atender_nuevo_esi(int serv_socket)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);

	int new_client_sock = accept(serv_socket, (struct sockaddr *)&client_addr, &client_len);
	if (new_client_sock < 0) {
	  perror("accept()");
	  return -1;
	}

	printf("Acepté al esi:%d.\n", new_client_sock);

	int i;
	for (i = 0; i < MAX_CLIENTES; ++i) {

		if (conexiones_esi[i].socket == NO_SOCKET) {
			conexiones_esi[i].socket = new_client_sock;
			conexiones_esi[i].addres = client_addr;
	        return new_client_sock;
	    }

	 }

	 // printf("Demasiadas conexiones. Cerrando nueva conexion %s:%d.\n", client_ipv4_str, client_addr.sin_port);
	 close(new_client_sock);

	 return -1;

}

void inicializar_conexiones_esi(void)
{
	for (int i = 0; i < MAX_CLIENTES; ++i)
	{
		conexiones_esi[i].socket = NO_SOCKET;
	}

}

int recibir_mensaje_esi(int esi_socket)
{
	int read_size;
	char client_message[2000];

	read_size = recv(esi_socket , client_message , 2000 , 0);
	if(read_size > 0)
	{
		printf("Esi %d dice: %s\n",esi_socket,client_message);
		int res_send = send(esi_socket, client_message, sizeof(client_message), 0);
	}


	return read_size;
}

int cerrar_conexion_esi(t_conexion_esi * esi)
{

	printf("Conexion con esi %d cerrada\n",esi->socket);
	close(esi->socket);
	esi->socket = NO_SOCKET;

	return 0;
}

int read_from_stdin(char *read_buffer, size_t max_len)
{
	char c = '\0';
	int i = 0;

	memset(read_buffer, 0, max_len);

	ssize_t read_count = 0;
	ssize_t total_read = 0;

	do{
		read_count = read(STDIN_FILENO, &c, 1);
		if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		  perror("read()");
		  return -1;
		}
		else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		  break;
		}
		else if (read_count > 0) {
		  total_read += read_count;

		  read_buffer[i] = c;
		  i++;

		  if (total_read > max_len) {
			//printf("Message too large and will be chopped. Please try to be shorter next time.\n");
			fflush(STDIN_FILENO);
			break;
		  }
		}
	}while ((read_count > 0) && (total_read < max_len) && (c != '\n'));

	size_t len = strlen(read_buffer);
	if (len > 0 && read_buffer[len - 1] == '\n')
		read_buffer[len - 1] = '\0';

	//printf("Read from stdin %zu bytes. Let's prepare message to send.\n", strlen(read_buffer));

	return 0;
}

void stdin_no_bloqueante(void)
{
	  /* Set nonblock for stdin. */
	  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
	  flag |= O_NONBLOCK;
	  fcntl(STDIN_FILENO, F_SETFL, flag);

}

void crear_listas_planificador(void)
{
	l_listos = list_create();
	l_bloqueados = list_create();
	l_terminados = list_create();
}

int recibir_mensaje_coordinador(int coord_socket)
{

	int read_size;
	char client_message[2000];

	read_size = recv(coord_socket , client_message , 2000 , 0);
	if(read_size > 0)
	{
		printf("Coordinador %d dice: %s\n",coord_socket,client_message);
		int res_send = send(coord_socket, client_message, sizeof(client_message), 0);
	}


	return read_size;

}

int cerrar_conexion_coord(int coord_socket)
{

	printf("Conexion con coordinador %d cerrada\n",coord_socket);
	close(coord_socket);

	return 0;
}
