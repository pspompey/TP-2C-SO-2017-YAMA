#ifndef MASTER_H_
#define MASTER_H_

#include <socket.h>
#include <commons/string.h>
#include <file.h>
#include <mlist.h>
#include <thread.h>

enum {TRANSFORMACION, REDUCCION_LOCAL, REDUCCION_GLOBAL, ALMACENAMIENTO};

typedef struct{
	thread_t* hilo;
	int etapa;
	bool active;
	int result;
} t_hilos;

struct{
	char* path_transf;
	char* path_reduc;
	char* arch;
	char* arch_result;
} job;

struct{
	t_file* fd_transf;
	char* script_transf;
	t_file* fd_reduc;
	char* script_reduc;
} script;

bool job_active;
mlist_t* hilos;
pthread_mutex_t mutex_hilos;

struct{
	int total;
	int transf;
	int reducc;
}tareasParalelo;

t_socket yama_socket;

time_t job_init;
time_t job_end;

#endif /* MASTER_H_ */
