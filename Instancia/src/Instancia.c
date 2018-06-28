/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Instancia.h"
#include "FuncionesInstancia.c"

int conexionConCoordinador() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons((int) PUERTO_COORDINADOR);

	int socketCoordinador = socket(AF_INET, SOCK_STREAM, 0);

	printf("Conectando con Coordinador...\n");
	if (connect(socketCoordinador, (void*) &direccionServidor,
			sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador\n");
		return 1;
	} else
		printf("Conectado correctamente.\n\n");
	return socketCoordinador;
}

t_respuesta_instancia * armarRespuestaParaCoordinador(int resultadoEjecucion) {
	int entradasDisponibles = obtenerEntradasDisponibles();

	t_respuesta_instancia * resultadoAEnviar = malloc(
			sizeof(t_respuesta_instancia));

	resultadoAEnviar->entradas_libres = entradasDisponibles;
	resultadoAEnviar->rdo_operacion = resultadoEjecucion;

	printf("Resultado a enviar:\n");
	printf("\tEntradas disponibles: %d\n", resultadoAEnviar->entradas_libres);
	printf("\tResultado de operacion: %d\n", resultadoAEnviar->rdo_operacion);

	return resultadoAEnviar;

}

void enviarResultadoSentencia(int socketCoordinador, int keyword) {
	printf("Envio de header para respuesta de sentencia...\n");

	int resultado;
	int resultadoEjecucion;

	switch (keyword) {
	t_respuesta_instancia * resultadoAEnviar;
case SET_:
	enviarHeader(socketCoordinador, instancia, coordinador,
	INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

	printf("Enviando Respuesta de sentencia SET...\n");

	resultadoEjecucion = EXITO_I;

	resultadoAEnviar = armarRespuestaParaCoordinador(resultadoEjecucion);

	resultado = send(socketCoordinador, resultadoAEnviar,
			sizeof(t_respuesta_instancia), 0);

	if (resultado == -1) {
		printf("Error en el send");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf("\tResultado: %d\n", resultado);
	printf("\tResultado de ejecucion enviado: %d\n", resultadoEjecucion);
	printf("--------------------------------------------------------\n");
	break;

case STORE_:
	enviarHeader(socketCoordinador, instancia, coordinador,
	INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));
	printf("Enviando Respuesta de sentencia STORE...\n");

	resultadoEjecucion = EXITO_I;

	resultadoAEnviar = armarRespuestaParaCoordinador(resultadoEjecucion);

	resultado = send(socketCoordinador, resultadoAEnviar,
			sizeof(t_respuesta_instancia), 0);

	if (resultado == -1) {
		printf("Error en el send");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf("\tResultado: %d\n", resultado);
	printf("\tResultado de ejecucion enviado: %d\n", resultadoEjecucion);
	printf("--------------------------------------------------------\n");
	break;

case RECONEXION:
	enviarHeader(socketCoordinador, instancia, coordinador,
	INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));
	printf(
			"Enviando Respuesta de Reconexion (cantidad de entradas disponibles)...\n");

	resultadoEjecucion = EXITO_I;

	resultadoAEnviar = armarRespuestaParaCoordinador(resultadoEjecucion);

	resultado = send(socketCoordinador, resultadoAEnviar,
			sizeof(t_respuesta_instancia), 0);

	if (resultado == -1) {
		printf("Error en el send");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf("\tResultado: %d\n", resultado);
	printf("\tResultado de ejecucion enviado: %d\n", resultadoEjecucion);
	printf("--------------------------------------------------------\n");
	break;

case OBTENER_VALOR:
	enviarHeader(socketCoordinador, instancia, coordinador,
	INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, strlen(valorConsultado));

	printf("Enviando Valor asociado a la clave consultada...\n");
	resultado = send(socketCoordinador, valorConsultado,
			strlen(valorConsultado), 0);

	if (resultado == -1) {
		printf("Error en el send");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf("\tResultado: %d\n", resultado);
	printf("\tValor obtenido y enviado: %s\n", valorConsultado);
	printf("--------------------------------------------------------\n");
	break;

default:
	break;
	}
}

t_configTablaEntradas * obtenerConfigTablaEntradas(int socketCoordinador) {

	t_configTablaEntradas * configRecibida = malloc(
			sizeof(t_configTablaEntradas));

	int statusHeader = recv(socketCoordinador, configRecibida,
			sizeof(t_configTablaEntradas), (int) NULL);

	if (statusHeader == -1) {
		printf("Error en el recv");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf(
			"################################################################\n");
	printf("Configuracion inicial de la Tabla de Entradas:\n");
	printf("\tCantidad total de entradas: %d\tTamaño de entradas:%d\n",
			configRecibida->cantTotalEntradas, configRecibida->tamanioEntradas);
	printf(
			"################################################################\n");

	return configRecibida;
}

void obtenerClavesARecuperar(int socketCoordinador, int cantClaves) {
	printf("Total de claves a recuperar: %d\n", cantClaves);

	char * clave = malloc(40);

	for (int i = 0; i < cantClaves; i++) {
		recv(socketCoordinador, clave, 40, 0);
		printf("[%d] Clave recibida para recuperar: %s\n", i + 1, clave);

		recuperarClave(clave);
	}

	free(clave);
}

void crearTablaEntradas(t_configTablaEntradas * config) {

	l_indice_entradas = list_create();

	int tamanioTotal = config->cantTotalEntradas * config->tamanioEntradas;

	tablaEntradas = malloc(tamanioTotal);

	// Esta linea es para limpiar la memoria que se esta reservando
	memset(tablaEntradas, 0, tamanioTotal);

	if (tablaEntradas != NULL) {
		printf("\tAlocado memoria para guardada de Valores\n");

	} else {
		printf("\tNo se pudo alocar la memoria para el guardado de Valores\n");
	}
}

t_sentencia * recibirSentencia(int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

	t_esi_operacion_sin_puntero * sentenciaPreliminarRecibida = malloc(
			sizeof(t_esi_operacion_sin_puntero));

	int statusSentenciaPreliminar = recv(socketCoordinador,
			sentenciaPreliminarRecibida, sizeof(t_esi_operacion_sin_puntero),
			(int) NULL);

	if (statusSentenciaPreliminar == -1) {
		printf("Error en el recv");
		// TODO: Implementar exit para abortar ejecucion
	}

	++contadorOperacion;

	printf("Status Sentencia Preliminar: %d \n", statusSentenciaPreliminar);
	printf("Sentencia preliminar recibida: \n");
	printf("\tKeyword: %d - Clave: %s - Tamanio del Valor: %d\n",
			sentenciaPreliminarRecibida->keyword,
			sentenciaPreliminarRecibida->clave,
			sentenciaPreliminarRecibida->tam_valor);

	if (sentenciaPreliminarRecibida->keyword == SET_) {
		// Se obtiene el valor y se forma la sentencia completa
		char * valorRecibido = recibirValor(socketCoordinador,
				sentenciaPreliminarRecibida->tam_valor);
		sentenciaRecibida = construirSentenciaConValor(
				sentenciaPreliminarRecibida, valorRecibido);
	}

	else {
		sentenciaRecibida = construirSentenciaConValor(
				sentenciaPreliminarRecibida, NULL);
	}

	return sentenciaRecibida;
}

int cantidadDeEntradasRequeridasParaValor(int tamanioTotalValor) {
	int entradasNecesariasParaGuardarValor = tamanioTotalValor
			/ configTablaEntradas->tamanioEntradas;
	int resto = tamanioTotalValor % configTablaEntradas->tamanioEntradas;

	if (resto > 0) {
		entradasNecesariasParaGuardarValor++;
	}

	return entradasNecesariasParaGuardarValor;
}

t_indice_entrada * guardarIndiceAtomicoEnTabla(char clave[40], char * valor,
		int nroEntrada) {

	t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
	strcpy(indiceEntrada->clave, clave);

	printf("Verificando si el indice ya contiene una entrada...\n");
	if (entradaExistenteEnIndice(nroEntrada)) {
		// TODO en lugar de puntero a char debe ser un char[40]
		char * clave = obtenerClaveExistenteEnEntrada(nroEntrada,
				l_indice_entradas);
		printf("Eliminando todas las entradas asociadas a la clave: %s...\n",
				clave);
		eliminarEntradasAsociadasAClave(clave);
	}

	indiceEntrada->numeroEntrada = nroEntrada;
	numeroEntrada++;

	indiceEntrada->esAtomica = true;
	indiceEntrada->nroDeOperacion = contadorOperacion;
	indiceEntrada->tamanioValor = strlen(valor);

	indiceEntrada->puntero = tablaEntradas
			+ (indiceEntrada->numeroEntrada
					* configTablaEntradas->tamanioEntradas);

	list_add(l_indice_entradas, indiceEntrada);

	printf("Indice agregado correctamente\n");

	return indiceEntrada;
}

t_indice_entrada * guardarEnEntradaDisponible(char clave[40], char * valor) {
	t_indice_entrada * indiceEntrada;

	int nroEntradaDisponible = buscarIndiceDeEntradaDisponible();

	indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
			nroEntradaDisponible);

	return indiceEntrada;
}

void guardarValorEnEntrada(char * valor, char* puntero) {
	printf("Guardando valor: %s en puntero: %p...\n", valor, puntero);
	memcpy(puntero, valor, strlen(valor));

	printf("Valor guardado: %s\n", puntero);
}

void ordenarAscPorCodDeOperacion(t_list * lista) {
	// El comparador devuelve si el primer parametro debe aparecer antes que el segundo en la lista
	_Bool menorCodigoDeOperacion(t_indice_entrada * entrada1,
			t_indice_entrada * entrada2) {
		return (entrada1->nroDeOperacion <= entrada2->nroDeOperacion);
	}

	printf("Ordenando lista por Numero de Operacion (ascendentemente)...\n");
	list_sort(lista, (void*) menorCodigoDeOperacion);

	return;
}

t_indice_entrada * aplicarAlgoritmoDeReemplazo(char clave[40], char * valor) {
	printf("Aplicando algoritmo de reemplazo: %s\n", ALGORITMO_DE_REEMPLAZO);

	t_indice_entrada * indiceEntrada;

	if (strcmp(ALGORITMO_DE_REEMPLAZO, "CIRC") == 0) {
		numeroEntrada = 0;
		printf("El puntero fue posicionado en la entrada: %d\n", numeroEntrada);
		indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
				numeroEntrada);

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "LRU") == 0) {
		// TODO: Buscar entrada con menor nro de operacion y reemplazarla

		t_list * listaAux = list_duplicate(l_indice_entradas);

		ordenarAscPorCodDeOperacion(listaAux);

		t_indice_entrada * entradaMenosUsada = list_get(listaAux, 0);

		// TODO en lugar de puntero a char debe ser un char[40]
		char * claveAReemplazar = obtenerClaveExistenteEnEntrada(
				entradaMenosUsada->numeroEntrada, listaAux);

		printf("Clave a ser reemplazada: %s\n", claveAReemplazar);

		list_destroy(listaAux);

		t_list * indicesQueContienenClave = obtenerIndicesDeClave(
				claveAReemplazar);

		t_indice_entrada * entradaBase = list_get(indicesQueContienenClave, 0);

		indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
				entradaBase->numeroEntrada);

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "BSU") == 0) {
		printf("Algoritmo BSU aun no desarrollado\n");
	}

	return indiceEntrada;
}

t_indice_entrada * guardarIndiceNoAtomicoEnTabla(char clave[40], char * valor,
		int numeroEntrada) {

	t_indice_entrada * indiceBase;

	char * punteroBase = tablaEntradas
			+ (numeroEntrada * configTablaEntradas->tamanioEntradas);

	int tamanioTotalValor = strlen(valor);

	printf("\tTamanio total del Valor: %d\n", tamanioTotalValor);
	printf("\tTamanio maximo a guardar por entrada: %d\n",
			configTablaEntradas->tamanioEntradas);

	int entradasNecesariasParaGuardarValor =
			cantidadDeEntradasRequeridasParaValor(tamanioTotalValor);

	printf("\tGuardando un total de %d entradas...\n",
			entradasNecesariasParaGuardarValor);

	for (int i = 1; i <= entradasNecesariasParaGuardarValor; i++) {
		printf("Guardando %d entrada requerida\n", i);

		t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
		strcpy(indiceEntrada->clave, clave);

		// validar que no exceda canntidad total de entradas
		indiceEntrada->numeroEntrada = numeroEntrada;
		printf("\tGuardando entrada en indice: %d\n",
				indiceEntrada->numeroEntrada);
		numeroEntrada++;

		indiceEntrada->esAtomica = false;
		indiceEntrada->nroDeOperacion = contadorOperacion;

		if (tamanioTotalValor > configTablaEntradas->tamanioEntradas) {
			indiceEntrada->tamanioValor = configTablaEntradas->tamanioEntradas;
		} else {
			indiceEntrada->tamanioValor = tamanioTotalValor;
		}
		tamanioTotalValor = tamanioTotalValor - indiceEntrada->tamanioValor;
		printf("\tTamanio de valor a guardar en indice: %d\n",
				indiceEntrada->tamanioValor);
		printf(
				"\t\tAun queda pendiente guardar %d del tamanio total del valor\n",
				tamanioTotalValor);

		indiceEntrada->puntero = tablaEntradas
				+ (indiceEntrada->numeroEntrada
						* configTablaEntradas->tamanioEntradas);
		printf("\tPuntero: %p\n", indiceEntrada->puntero);

		list_add(l_indice_entradas, indiceEntrada);

		printf("Indice agregado correctamente\n");

		// Asignando como indice Base el primer indice correspondiente a la clave
		if (i == 1) {
			indiceBase = indiceEntrada;
		}

	}

	return indiceBase;
}

void actualizarEntradasConNuevoValor(char clave[40], char * valor) {
	t_list * indices = obtenerIndicesDeClave(clave);
	t_indice_entrada * indiceBase = list_get(indices, 0);
	eliminarEntradasAsociadasAClave(clave);

	// Me guardo el numero de entrada para no perderlo al guardar claveValor
	int nroEntradaAux = numeroEntrada;

	// Seteo el numero de entrada con el de las claves ya existentes.
	numeroEntrada = indiceBase->numeroEntrada;
	guardarClaveValor(clave, valor);

	// Recupero el numero de entrada
	numeroEntrada = nroEntradaAux;
}

void guardarClaveValor(char clave[40], char * valor) {

	t_list * indicesQueContienenClave = obtenerIndicesDeClave(clave);
	int cantidadDeIndices = list_size(indicesQueContienenClave);
	printf("Cantidad de indices que contienen clave: %d\n", cantidadDeIndices);

	if (cantidadDeIndices > 0) {
		printf("La clave '%s' ya existe. Reemplazar entrada/s...\n", clave);

		int entradasNecesarias = cantidadDeEntradasRequeridasParaValor(
				strlen(valor));

		if (entradasNecesarias > cantidadDeIndices) {
			printf(
					"SET no podra hacer que se ocupen mas entradas (enunciado)\n");
			// TODO: devolver error al Coordinador
		} else {
			printf("Actualizando entradas existentes con nuevo valor: %s\n",
					valor);
			actualizarEntradasConNuevoValor(clave, valor);
		}

		actualizarNroDeOperacion(indicesQueContienenClave);
	} else {
		printf("La clave no existe... guardar...\n");

		int entradasNecesariasParaGuardarValor =
				cantidadDeEntradasRequeridasParaValor(strlen(valor));

		if (entradasNecesariasParaGuardarValor > 1) {
			// Guardar valor NO atomico en varias entradas

			int entradasDisponibles = obtenerEntradasDisponibles();

			if (entradasDisponibles >= entradasNecesariasParaGuardarValor) {

				t_indice_entrada * indiceBase = guardarIndiceNoAtomicoEnTabla(
						clave, valor, numeroEntrada);

				numeroEntrada = numeroEntrada
						+ entradasNecesariasParaGuardarValor;

				// guardarValorEnEntrada(sentenciaRecibida->valor,	indiceEntrada->puntero);

				printf("Guardando valor: %s en puntero: %p...\n", valor,
						indiceBase->puntero);
				memcpy(indiceBase->puntero, valor, strlen(valor));

				printf("Valor guardado: %s\n", indiceBase->puntero);

			} else {
				printf("No hay mas lugar para guardar un valor NO atomico.\n");
				//TODO: Devolver un error al coordinador
			}

		} else {

			printf("Agregando unica entrada, es decir valor atomico\n");

			t_indice_entrada * indiceEntrada;

			if (numeroEntrada >= configTablaEntradas->cantTotalEntradas) {
				int entradasDisponibles = obtenerEntradasDisponibles();

				if (entradasDisponibles != 0) {
					indiceEntrada = guardarEnEntradaDisponible(clave, valor);
				} else {
					printf("Es necesario aplicar algoritmo de reemplazo...\n");
					indiceEntrada = aplicarAlgoritmoDeReemplazo(clave, valor);
				}
			} else {
				printf("No es necesario aplicar algoritmo de reemplazo\n");
				indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
						numeroEntrada);
			}

			// guardarValorEnEntrada(sentenciaRecibida->valor,indiceEntrada->puntero);

			printf("Guardando valor: %s en puntero: %p...\n", valor,
					indiceEntrada->puntero);
			memcpy(indiceEntrada->puntero, valor, strlen(valor));

			printf("Valor guardado: %s\n", indiceEntrada->puntero);
		}

		imprimirTablaEntradas();

		printf(
				"TODO: Se supera la cantidad maxima de entradas definida por el coordinador. Se requiere reemplazar o compactar\n");
	}
}

void make_directory(const char* name) {
//char * check;
	int check;
	check = mkdir(name, 0777);

	if (!check)
		printf("Directorio creado: %s\t check = %d\n", name, check);

	else {
		printf(
				"No se puede crear el directorio... o ya existe...\t check = %d\n",
				check);
	}
}

char * obtenerPathArchivo(char clave[40]) {
	char * puntoDeMontaje = strdup(PUNTO_DE_MONTAJE);
	printf("Directorio donde se guardara el archivo: %s\n", puntoDeMontaje);

	char * archivo;
	archivo = string_new();
	string_append(&archivo, clave);
	string_append(&archivo, ".txt");
	printf("Nombre del archivo que se creara: %s\n", archivo);

	char * path = string_new();
	string_append(&path, puntoDeMontaje);
	string_append(&path, archivo);
	printf("Path del archivo a crear: %s\n", path);

	return path;
}

size_t getFileSize(const char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}

// &len
void * append(int fd, char * valor, size_t nbytes, void *map, size_t len) {
	int permisos = PROT_READ | PROT_WRITE;
	ssize_t written = write(fd, valor, nbytes);
	munmap(map, len);
	len += written;
	return mmap(NULL, len, permisos, MAP_SHARED, fd, 0);
}

void grabarArchivoPorPrimeraVez(int fd, char * valor, int tamanio) {
	write(fd, valor, tamanio);
}

void recuperarClave(char clave[40]) {
	printf("Clave a recuperar: %s\n", clave);

	char * path = obtenerPathArchivo(clave);

	int permisos = 0664;
	int flags = O_RDONLY;

	int fileDescriptor;
	if ((fileDescriptor = open(path, flags, permisos)) < 0) {
		printf("No se pudo crear el archivo: %s\n", path);
		printf("\tFile Descriptor: %d\n", fileDescriptor);
	} else {
		printf("File Descriptor: %d\n", fileDescriptor);

		char * valor = malloc(PACKAGESIZE);

		int tamanioValor = read(fileDescriptor, valor, PACKAGESIZE);

		printf("Valor recuperado: %s\n", valor);
		printf("Tamaño del Valor recuperado: %d\n", tamanioValor);

		guardarClaveValor(clave, valor);

		free(valor);

		printf("Cerrando el fileDescriptor...\n");
		if (close(fileDescriptor) == 0) {
			printf("\tArchivo cerrado correctamente.\n");
		} else {
			printf("\tError en el cerrado del archivo.\n");
		}
	}
}

void obtenerValorAsociadoAClave(char clave[40]) {
	printf("Obteniendo valor asociado a la clave: %s\n", clave);
	t_list * listaIndices  =  obtenerIndicesDeClave(clave);
	int tamanioValor = obtenerTamanioTotalDeValorGuardado(listaIndices);
	t_indice_entrada * indiceBase = list_get(listaIndices, 0);
	char * valor = obtenerValorCompleto(indiceBase->puntero, tamanioValor);
	strcpy(valorConsultado, valor);
}

void realizarStoreDeClave(char clave[40]) {
	char * path = obtenerPathArchivo(clave);

	make_directory(PUNTO_DE_MONTAJE);

	int permisos = 0664;
	int flags = O_RDWR | O_CREAT | O_TRUNC;

	int fileDescriptor;
	if ((fileDescriptor = open(path, flags, permisos)) < 0) {
		printf("No se pudo crear el archivo: %s\n", path);
		printf("\tFile Descriptor: %d\n", fileDescriptor);
	} else {
		printf("File Descriptor: %d\n", fileDescriptor);
	}

	t_list * indices = obtenerIndicesDeClave(clave);

	actualizarNroDeOperacion(indices);

	imprimirTablaEntradas();

	t_indice_entrada * primerEntrada = list_get(indices, 0);

	int tamanioValorCompleto = obtenerTamanioTotalDeValorGuardado(indices);
	printf("Tamaño del valor guardado: %d\n", tamanioValorCompleto);

	char * valorCompleto = obtenerValorCompleto(primerEntrada->puntero,
			tamanioValorCompleto);
	printf("El valor completo es: %s\n", valorCompleto);

	grabarArchivoPorPrimeraVez(fileDescriptor, valorCompleto,
			tamanioValorCompleto);

	char * punteroDeArchivo;

//  ACA APLICAR EL APPEND PARA MANEJAR LA VARIACION DEL TAMAÑO DEL FD
	if ((punteroDeArchivo = mmap(0, tamanioValorCompleto, permisos,
	MAP_SHARED, fileDescriptor, 0)) == (caddr_t) -1) {
		printf("mmap error for output\n");
	}

//	punteroDeArchivo = append(fileDescriptor, valor, tamanioDelValor, void *map, size_t len);

	if (punteroDeArchivo != MAP_FAILED) {
		printf("El Mapeo se efectuo correctamente\n\n");
		printf("Puntero de archivo: %p\n", punteroDeArchivo);
	} else {
		printf("Error en el Mapeo de archivo\n");
		switch (errno) {
		case EINVAL:
			printf(
					"\tEither address was unusable, or inconsistent flags were given.\n");
			break;
		case EACCES:
			printf(
					"\tfiledes was not open for the type of access specified in protect.\n");
			break;
		case ENOMEM:
			printf(
					"\tEither there is not enough memory for the operation, or the process is out of address space.\n");
			break;
		case ENODEV:
			printf("\tThis file is of a type that doesn't support mapping.\n");
			break;
		case ENOEXEC:
			printf(
					"\tThe file is on a filesystem that doesn't support mapping.\n");
			break;
		}
	}

	if (close(fileDescriptor) == 0) {
		printf("\tArchivo cerrado correctamente.\n");
	} else {
		printf("\tError en el cerrado del archivo.\n");
	}
}

void grabarArchivo(char clave[40]) {
	printf("Preparandose para grabar en archivo...\n");
// TODO: Remover los valores hardcodeados e implementar
	t_list * listaDeIndices = obtenerIndicesDeClave(clave);

	printf("Obteniendo indice base que contiene la clave %s\n", clave);
	t_indice_entrada * primerEntrada = list_get(listaDeIndices, 0);
	printf("\tEl numero de entrada es: %d\n", primerEntrada->numeroEntrada);

	char * puntero = malloc(sizeof(char *));
	puntero = primerEntrada->puntero;
	printf("Puntero donde se encuentra el valor: %p\n", puntero);

	int tamanio = obtenerTamanioTotalDeValorGuardado(listaDeIndices);

	char * path = obtenerPathArchivo(clave);

	char * addressOfNewMapping = (void *) mmap(0, tamanio, PROT_WRITE,
	MAP_SHARED, (int) path, 0);
	if ((int) addressOfNewMapping != -1) {
		printf("mmap se realizo correctamente y se guardo en %p",
				addressOfNewMapping);
	} else {
		printf("Error en el mmap\n");
	}
}

void interpretarOperacionCoordinador(t_content_header * header,
		int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_CONFIG_INICIAL:

		configTablaEntradas = obtenerConfigTablaEntradas(socketCoordinador);

		crearTablaEntradas(configTablaEntradas);

		break;

	case COORDINADOR_INSTANCIA_RECUPERAR_CLAVES:

		printf("Recibiendo claves a recuperar...\n");

		obtenerClavesARecuperar(socketCoordinador, header->cantidad_a_leer);

		enviarResultadoSentencia(socketCoordinador, RECONEXION); // TODO: Añadir al enum el keyword reconexion= 3

		break;

	case COORDINADOR_INSTANCIA_SENTENCIA:

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		switch (sentenciaRecibida->keyword) {

		case SET_:
			guardarClaveValor(sentenciaRecibida->clave,
					sentenciaRecibida->valor);
			imprimirTablaEntradas();
			enviarResultadoSentencia(socketCoordinador, SET_);
			break;

		case STORE_:
			realizarStoreDeClave(sentenciaRecibida->clave);
			enviarResultadoSentencia(socketCoordinador, STORE_);
			break;

		case GET_:
			printf(
					"El Keyword GET_ no tiene comportamiento alguno en la Instancia.\n");
			break;

		case OBTENER_VALOR:
			printf("TODO: Leer clave y devolver valor...\n");
			obtenerValorAsociadoAClave(sentenciaRecibida->clave);

			enviarResultadoSentencia(socketCoordinador, OBTENER_VALOR);
			break;
		}

		break;

	case COORDINADOR_INSTANCIA_COMPROBAR_CONEXION:
		printf("Confirmando a coordinador conexion activa de Instancia...\n");
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_CONFIRMA_CONEXION_ACTIVA, 0);
		break;
	}
}

int main(int argc, char **argv) {

	cargarArchivoDeConfig(argv[1]);

	int socketCoordinador = conexionConCoordinador();

	enviarNombreInstanciaACoordinador(socketCoordinador);

	t_content_header * header = malloc(sizeof(t_content_header));
	int status = 1;

	status = interpretarHeader(socketCoordinador, header);
	while (status != -1 && status != 0) {
		switch (header->proceso_origen) {

		case coordinador:
			interpretarOperacionCoordinador(header, socketCoordinador);
			break;

		default:
			printf(
					"El codigo de proceso no es correcto o no esta identificado.\n");
			break;
		}
		status = interpretarHeader(socketCoordinador, header);
	}

	close(socketCoordinador);

	return 0;
}
