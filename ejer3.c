#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// Imprime el nro de la senhal
void handler (int sig) {
    printf("Senhal nro %d recibida \n", sig);
}


int main (void) {
    // Funciones que reciben las senhales
    // Aqui se pueden agregar la cantidad de funciones necesarias
    // Para este caso he agregado solo algunas
    signal(SIGABRT, &handler);
    signal(SIGALRM, &handler);
    signal(SIGCONT, &handler);
    signal(SIGFPE, &handler);
    signal(SIGINT, &handler);
    signal(SIGQUIT, &handler);
    signal(SIGSEGV, &handler);
    signal(SIGTERM, &handler);
    signal(SIGUSR1, &handler);
    signal(SIGUSR2, &handler);

    // Loop para mantener corriendo el programa
    while (1) {
        printf("Esperando senhal... \n");
        sleep(5);
    }

    return 0;
}

