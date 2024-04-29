#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// Estructura para guardar la informacion de los procesos
typedef struct {
    pid_t pid;              // Identificador del proceso
    int signal;             // Senal a enviar
    int SegundosDelay;      // Tiempo de espera antes de enviar la senal
} InfoProceso;


// Funcion para programar y enviar las senales a los procesos
void programar_senales(InfoProceso procesos[], int num_procesos) {
    for (int i = 0; i < num_procesos; i++) {
        pid_t pid           = procesos[i].pid;
        int signal          = procesos[i].signal;
        int SegundosDelay   = procesos[i].SegundosDelay;
        
        // Esperar el tiempo de delay
        sleep(SegundosDelay);
        
        // Enviar la senal al proceso
        if (kill(pid, signal) == -1) {
            perror("señal no enviada");
        } else {
            printf("Señal %d enviada al proceso %d\n", signal, (int)pid);
        }
    }
}

int main(int argc, char *argv[]) {
    // Verifica que hayan 2 argumentos
    if (argc != 2) {        
        fprintf(stderr, "Formato incorreto\nFormato: %s <Archivo>\n", argv[0]);
        return 1;
    }

    // Abre el archivo
    char *archivo = argv[1];
    FILE *file = fopen(archivo, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    InfoProceso procesos[100];      // Arreglo para almacenar información sobre los procesos
    int num_procesos = 0;           // Contador para el número de procesos leídos


    // Lee información de los procesos desde el archivo y carga en el arreglo
    while (fscanf(file, "%d %d %d", &procesos[num_procesos].pid,
                                    &procesos[num_procesos].signal,
                                    &procesos[num_procesos].SegundosDelay) == 3) {
        num_procesos++;
    }

    fclose(file);

    // Programar y enviar las senales a los procesos
    programar_senales(procesos, num_procesos);

    return 0;
}
