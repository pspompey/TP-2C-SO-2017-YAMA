#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <sys/select.h>
#include <stddef.h>

typedef int socket_t;

typedef struct {
    fd_set set;
    socket_t max;
} fdset_t;

/**
 * Si se indica la IP, crea un socket y lo conecta al servidor de la IP y
 * puerto especificados. Si no se indica la IP, crea un socket y lo prepara
 * para escuchar conexiones en el puerto especificado.
 * @param ip Dirección IP del servidor.
 * @param port Puerto.
 * @return Descriptor del socket.
 */
socket_t socket_init(const char *ip, const char *port);

/**
 * Crea un socket de servidor para conectarse con un cliente a través de un
 * puerto determinado.
 * @param port Puerto de escucha.
 * @return Descriptor del socket del cliente.
 */
socket_t socket_listen(const char *port);

/*
 * Función bloqueante que espera por conexiones en un socket servidor y las
 * acepta devolviendo el socket cliente.
 * @param sv_sock Descriptor del socket del servidor.
 * @return Descriptor del socket del cliente (-1 si hubo error).
 */
socket_t socket_accept(socket_t sv_sock);

/**
 * Crea un socket de cliente para conectarse con un servidor en una dirección
 * IP y un puerto determinados.
 * @param ip Dirección IP del servidor.
 * @param port Puerto del servidor.
 * @return Descriptor del socket del servidor.
 */
socket_t socket_connect(const char *ip, const char *port);

/**
 * Envía una cadena de texto por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a enviar.
 * @return Número de bytes enviados.
 */
size_t socket_send_string(socket_t sockfd, const char *message);

/**
 * Envía datos binarios por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a enviar.
 * @param size Tamaño de los datos.
 * @return Número de bytes enviados.
 */
size_t socket_send_bytes(socket_t sockfd, const char *message, size_t size);

/**
 * Recibe una cadena de texto por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a recibir.
 * @return Número de bytes recibidos (-1 si hubo error).
 */
ssize_t socket_receive_string(socket_t sockfd, char *message);

/**
 * Recibe datos binarios por una conexión abierta en un determinado socket.
 * @param sockfd Descriptor del socket.
 * @param message Mensaje a recibir.
 * @param size Tamaño de los datos.
 * @return Número de bytes recibidos (-1 si hubo error).
 */
ssize_t socket_receive_bytes(socket_t sockfd, char *message, size_t size);

/**
 * Crea un conjunto de sockets para ser usado por socket_select().
 * @return Conjunto de sockets.
 */
fdset_t socket_set_create(void);

/**
 * Agrega un socket a un conjunto de sockets.
 * @param fds conjunto de sockets.
 * @param fd socket a agregar.
 */
void socket_set_add(fdset_t *fds, socket_t fd);

/**
 * Elimina un socket de un conjunto de sockets.
 * @param fds conjunto de sockets.
 * @param fd socket a eliminar.
 */
void socket_set_remove(fdset_t *fds, socket_t fd);

/**
 * Verifica si un socket está presente en un conjunto de sockets.
 * @param fds conjunto de sockets.
 * @param fd socket a verificar.
 */
int socket_set_contains(fdset_t *fds, socket_t fd);

/**
 * Cierra un socket abierto con socket_listen() o socket_connect().
 * @param sockfd Descriptor del socket a cerrar.
 */
void socket_close(socket_t sockfd);

///////////////////////////////////////

#include <netinet/in.h>

int crearSocket();
int obtenerSocketMaximoInicial(int socketYama, int socketDataNode);
void inicializarSOCKADDR_IN(struct sockaddr_in* direccion, char* direccionIP, char* puerto);
void reutilizarSocket(int socketFD);
void asignarDirecciones(int socketFD, const struct sockaddr* sockDIR);

#endif /* SOCKET_H_ */

