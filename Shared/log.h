#ifndef LOG_H_
#define LOG_H_

/**
 * Inicializa el log del proceso actual.
 */
void log_init(void);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje no se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_inform(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_print(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje de error.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_report(const char *format, ...);

/**
 * Termina el log del proceso actual.
 */
void log_term(void);

#endif /* LOG_H_ */
