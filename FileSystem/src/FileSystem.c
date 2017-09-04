#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "socket.h"
#include "serial.h"
#include <string.h>

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
int SocketBuscado_GLOBAL = 0;
t_log* logFileSystem;

typedef struct{
 char* IP_FILESYSTEM;
 char* PUERTO_WORKER;
 char* PUERTO_DATANODE;
 char* PUERTO_YAMA;

}tinformacion;

typedef struct {
 int id;
 int tamanio;
} t_cabecera;

//funciones
void escucharPuertosDataNodeYYama(tinformacion);
void handshake(int, struct sockaddr_in, fd_set,fd_set, int);
void inicializarSOCKADDR_IN(struct sockaddr_in*, char*,char*);
int crearSocket();
int obtenerSocketMaximoInicial(int, int);
void reutilizarSocket(int);
void asignarDirecciones(int, const struct sockaddr*);

int main() {
	char* path = "/home/utnso/archivoConfiguracion/archivoConfigFileSystem.cfg";
	t_config* archivoConfig = config_create(path);
	tinformacion informacion_socket;

	logFileSystem = log_create("LogKernel.log", "FileSystem", false,LOG_LEVEL_TRACE);
	//VALIDACIONES

		if (archivoConfig == NULL) {
			log_error(logFileSystem,
					"Archivo configuracion: error al intentar leer ruta");
			return 1;
		}
		if (!config_has_property(archivoConfig, "IP_FILESYSTEM")) {
			log_error(logFileSystem,
					"Archivo configuracion: error al leer IP_FILESYSTEM");
			return 1;
		}
		if (!config_has_property(archivoConfig, "PUERTO_WORKER")) {
			log_error(logFileSystem, "Archivo configuracion: error al leer PUERTO_WORKER");
		}
		if (!config_has_property(archivoConfig, "PUERTO_DATANODE")) {
					log_error(logFileSystem, "Archivo configuracion: error al leer PUERTO_DATANODE");
		}
		if (!config_has_property(archivoConfig, "PUERTO_YAMA")) {
							log_error(logFileSystem, "Archivo configuracion: error al leer PUERTO_YAMA");
		}
		//fin VALIDACIONES
		//configuracion del archivo configuracion
		informacion_socket.IP_FILESYSTEM = config_get_string_value(archivoConfig, "IP_FILESYSTEM");
		informacion_socket.PUERTO_DATANODE = config_get_string_value(archivoConfig, "PUERTO_DATANODE");
		informacion_socket.PUERTO_WORKER = config_get_string_value(archivoConfig, "PUERTO_WORKER");
		informacion_socket.PUERTO_YAMA = config_get_string_value(archivoConfig, "PUERTO_YAMA");

		escucharPuertosDataNodeYYama(informacion_socket);
}


void escucharPuertosDataNodeYYama(tinformacion informacion_socket) {

	fd_set master;
	fd_set read_fds;
	int maximo_Sockets;
	//datanode
	int socketEscuchandoDataNode;
	struct sockaddr_in direccion_DataNode;

	//crea socket para el datanode
	inicializarSOCKADDR_IN(&direccion_DataNode,informacion_socket.IP_FILESYSTEM,informacion_socket.PUERTO_DATANODE);
	socketEscuchandoDataNode = crearSocket(); //abstraccion, se debe armar la funcion

	//Yama
	struct sockaddr_in direccion_Yama;
	int socketEscuchandoYama;

	//crea socket para el yama
	inicializarSOCKADDR_IN(&direccion_Yama,informacion_socket.IP_FILESYSTEM,informacion_socket.PUERTO_YAMA);
	socketEscuchandoYama = crearSocket();

	//socket servidor
	reutilizarSocket(socketEscuchandoDataNode); // obviar el mensaje "address already in use"
	asignarDirecciones(socketEscuchandoDataNode,
			(struct sockaddr *) &direccion_DataNode); // asignamos el Struct de las direcciones al socket
	reutilizarSocket(socketEscuchandoYama);
	asignarDirecciones(socketEscuchandoYama, (struct sockaddr *) &direccion_Yama);

	//Preparo para escuchar los 2 puertos en el master
	printf("Esperando conexiones entrantes \n");
	FD_ZERO(&master);
	FD_SET(socketEscuchandoDataNode, &master);
	FD_SET(socketEscuchandoYama, &master);
	maximo_Sockets = obtenerSocketMaximoInicial(socketEscuchandoYama,socketEscuchandoDataNode);

	if (listen(socketEscuchandoDataNode, 10) == -1) {
		perror("listen");
		exit(1);
	}
	if (listen(socketEscuchandoYama, 10) == -1) {
		perror("listen");
		exit(1);
	}
	//Comienza el Select
	for (;;) {
		int nuevoSocket;
		FD_ZERO(&read_fds);
		struct sockaddr_in remoteaddr;
		socklen_t addrlen;
		read_fds = master; // cópialo
		if (select((maximo_Sockets) + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(socketEscuchandoDataNode, &read_fds)) { // ¡¡tenemos datos de una datanode!!
			printf("Escucho algo por datanode \n");
			addrlen = sizeof(remoteaddr);
			if ((nuevoSocket = accept(socketEscuchandoDataNode,
					(struct sockaddr *) &remoteaddr, (socklen_t *) &addrlen))
					== -1) {
				perror("Error de aceptacion de un datanode \n");
			} else {
				FD_SET(nuevoSocket, &master); // añadir al conjunto maestro
				FD_SET(nuevoSocket, &read_fds);
				maximo_Sockets =
						(maximo_Sockets < nuevoSocket) ?
								nuevoSocket : maximo_Sockets;
				printf("Ya acepte un datanode \n ");
				handshake(nuevoSocket, remoteaddr, master, read_fds,
						maximo_Sockets);
				//ejecuto lo que tengo que hacerr para el datanodee....
			}
		} else if (FD_ISSET(socketEscuchandoYama, &read_fds)) { // ¡¡tenemos datos de un Yama!!
			addrlen = sizeof(remoteaddr);

			////////// Debe estar en estado estable el filesystem para poder aceptar conexion de un YAMA


			if ((nuevoSocket = accept(socketEscuchandoYama,
					(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
				perror("Error de aceptacion de Yama \n");
			} else {
				FD_SET(nuevoSocket, &master); // añadir al conjunto maestro
				FD_SET(nuevoSocket, &read_fds);
				maximo_Sockets =
						(maximo_Sockets < nuevoSocket) ?
								nuevoSocket : maximo_Sockets;
				handshake(nuevoSocket, remoteaddr, master, read_fds,
						maximo_Sockets);
				//ejecuto rutina con yama

			}
		}

		else {
			/*Si no entro en los if anteriores es por que no es conexion nueva por
			 ninguno de los dos sockets, por lo tanto es algun socket descriptor
			 ya almacenado que tiene actividad.
			 luego gestionar mensaje*/
			int socketFor;
			int bytes;
			t_cabecera header;
			for (socketFor = 0; socketFor < (maximo_Sockets + 1); socketFor++) {
				if (FD_ISSET(socketFor, &read_fds)) {
					if ((bytes = recv(socketFor, &header, sizeof(header), 0)) //recibe un mensaje el cual en caso de error es menor a cero y se limpian el FD y se cierra el socket
							<= 0) {
						FD_CLR(socketFor, &read_fds);
						FD_CLR(socketFor, &master);
						close(socketFor);
					} else {
						///switch con mensajes
					}
					}
			}

}
}
}


void handshake(int nuevoSocket, struct sockaddr_in remoteaddr, fd_set master,
		fd_set read_fds, int maximo_Sockets) {
	char buffer_select[MAXIMO_TAMANIO_DATOS];
	int cantidadBytes;

	cantidadBytes = recv(nuevoSocket, buffer_select, sizeof(buffer_select), 0);
	buffer_select[cantidadBytes] = '\0';
	printf("Credencial recibida: %s\n", buffer_select);
	if (!strcmp(buffer_select, "Yatpos\0")) { //Yatpos es la credencial que autoriza al proceso a seguir conectado
		printf("FileSystem: nueva conexion desde %s en socket %d\n",
				inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
		if (send(nuevoSocket, "Bienvenido al FileSystem!", 26, 0) == -1) {
			perror("Error en el send");
		}
	} else {
		printf(
				"El proceso que requirio acceso, no posee los permisos adecuados\n");
		if (send(nuevoSocket, "Usted no esta autorizado!", MAXIMO_TAMANIO_DATOS,
				0) == -1) {
			perror("Error en el send");
		}
		close(nuevoSocket);
	}
}


int crearSocket() // funcion para crear socket
{
	int socketfd;
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //inicializamos el socket
			{
		perror("Socket Fail");
		exit(1);
	}
	return socketfd; //devolvemos el socket creado
}



int obtenerSocketMaximoInicial(int socketYama, int socketDataNode) {
	int socketMaximoInicial = 0;

	if (socketYama > socketDataNode) {
		socketMaximoInicial = socketYama;
	} else {
		socketMaximoInicial = socketDataNode;
	}
	return socketMaximoInicial;
}

void inicializarSOCKADDR_IN(struct sockaddr_in* direccion, char* direccionIP,
		char* puerto) // La funcion transforma sola los datos de host a network
{
	direccion->sin_family = AF_INET;
	direccion->sin_addr.s_addr = inet_addr(direccionIP);
	direccion->sin_port = htons(atoi(puerto));
	memset(&(direccion->sin_zero), '/0', 8);
	return;
}

void reutilizarSocket(int socketFD) {
	int yes = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) //el socket se puede reutilizar
			{
		perror("setsockopt");
		exit(2);
	}
	return;
}

void asignarDirecciones(int socketFD, const struct sockaddr* sockDIR) //Asociamos el puerto y direccion al socket
{
	if (bind(socketFD, sockDIR, sizeof(struct sockaddr)) == -1) {
		perror("Bind fail");
		exit(3);
	}
	return;
}