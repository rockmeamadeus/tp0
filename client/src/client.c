#include "client.h"

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

	int conexion = -1; // inválido hasta conectar
	char *ip;
	char *puerto;
	char *valor;

	t_log *logger;
	t_config *config;

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger();

	// Usando el logger creado previamente
	// Escribi: "Hola! Soy un log"
	log_info(logger, "Hola! Soy un log");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config();

	// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	valor = config_get_string_value(config, "CLAVE");

	// Comprobamos que las claves existan
	if (ip == NULL || puerto == NULL || valor == NULL)
	{
		log_error(logger, "Falta una clave en cliente.config (IP/PUERTO/CLAVE)");
		terminar_programa(conexion, logger, config);
		abort();
	}

	// Loggeamos el valor de config
	log_info(logger, "IP: %s, Puerto: %s, Valor: %s", ip, puerto, valor);

	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	// ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

	// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	if (conexion < 0)
	{
		log_error(logger, "No se pudo crear la conexión a %s:%s", ip, puerto);
		terminar_programa(conexion, logger, config);
		abort();
	}

	// Enviamos al servidor el valor de CLAVE como mensaje

	enviar_mensaje(valor, conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
}

t_log *iniciar_logger(void)
{
	t_log *logger = log_create("tp0.log", "CLIENTE", true, LOG_LEVEL_INFO);
	if (logger == NULL)
	{
		fprintf(stderr, "No se pudo crear el logger\n");
		abort();
	}
	return logger;
}

t_config *iniciar_config(void)
{
	t_config *nuevo_config = config_create("cliente.config");
	if (nuevo_config == NULL)
	{
		fprintf(stderr, "No se pudo crear el config (cliente.config)\n");
		abort();
	}
	return nuevo_config;
}

void leer_consola(t_log *logger)
{
	char *leido;

	// Primera lectura
	leido = readline("> ");

	while (leido != NULL && strlen(leido) > 0)
	{
		log_info(logger, "Leído de consola: %s", leido);
		free(leido);
		leido = readline("> ");
	}

	// Liberamos si no es NULL (puede serlo si fue Ctrl+D)
	if (leido != NULL)
		free(leido);
}

void paquete(int conexion)
{
    char *leido;
    t_paquete *paquete;

    paquete = crear_paquete();
    if (paquete == NULL)
    {
        return;
    }

    int cantidad = 0;

    leido = readline("> ");
    while (leido != NULL && strlen(leido) > 0)
    {
        agregar_a_paquete(paquete, leido, strlen(leido) + 1);
        cantidad++;
        free(leido);
        leido = readline("> ");
    }

    if (leido != NULL)
        free(leido);

    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
	  con las funciones de las commons y del TP mencionadas en el enunciado */
	if (conexion >= 0)
	{
		liberar_conexion(conexion);
	}
	if (logger != NULL)
	{
		log_destroy(logger);
	}
	if (config != NULL)
	{
		config_destroy(config);
	}
}
