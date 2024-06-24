#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Constantes y variables globales
#define MAX_AUTOS 100          // Máximo número de autos en espera en cada dirección
#define MAX_ON_BRIDGE 3        // Máximo número de autos en el puente a la vez
#define MAX_CONSECUTIVE 4      // Máximo número de autos consecutivos que pueden cruzar en una dirección

pthread_mutex_t bridge_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para sincronizar el acceso al puente
pthread_cond_t turn_cond = PTHREAD_COND_INITIALIZER;      // Variable de condición para alternar turnos

typedef struct {
    char id[7]; // Estructura para almacenar el ID de un auto
    char direccion[4];
} Car;

Car cars[100];
char cars_crossed_direction[100][4];
Car cars_right[MAX_AUTOS]; // Array para autos esperando a ir hacia la derecha
Car cars_left[MAX_AUTOS];  // Array para autos esperando a ir hacia la izquierda
int waiting_right = 0, waiting_left = 0; // Contadores de autos esperando en cada dirección
int on_bridge_right = 0, on_bridge_left = 0; // Contadores de autos en el puente en cada dirección
int consecutive_right = 0, consecutive_left = 0; // Contadores de autos consecutivos en cada dirección
int car_count = 0;
int cars_crossed = 0;
int waiting_right_copy = 0, waiting_left_copy = 0;

// variables para funcion start
static char auto01[7];
static char auto02[7];
static char auto03[7];
static int cars_printed_der = 0;
static int cars_printed_izq = 0;
int cars_count_copy = 0;


// Función para agregar un auto en la dirección especificada
void add_car(char *direction) {
    Car new_car;
    sprintf(new_car.id, "auto%02d", ++cars_count_copy);     // Generar ID único para el auto
    strcpy(new_car.direccion, direction);                   // Asignar dirección al auto
    car_count++;                                            // Incrementar el tamaño lógico del array
    strcpy(cars[car_count-1].id, new_car.id);
    strcpy(cars[car_count-1].direccion, new_car.direccion);

    if (strcmp(direction, "der") == 0) {
        cars_right[waiting_right++] = new_car; // Agregar auto a la cola derecha
        waiting_right_copy++;
    } else if (strcmp(direction, "izq") == 0) {
        cars_left[waiting_left++] = new_car; // Agregar auto a la cola izquierda
        waiting_left_copy++;
    }

    pthread_mutex_unlock(&bridge_mutex);
    pthread_cond_signal(&turn_cond); // Notificar a los autos en espera que hay un cambio
}

void delete_car(Car cars[], int *num_cars, int index) {
    // Mover los elementos después del índice a una posición a la izquierda
    for (int i = index; i < *num_cars - 1; i++) {
        cars[i] = cars[i + 1];
    }
    // Decrementar el tamaño lógico del array
    (*num_cars)--;
}

// Función para simular el cruce del puente
void cross_bridge(void *arg) {
    Car *car = (Car *)arg;
        //printf("HOLAA soy %s y me voy a %s\n", car->id, car->direccion);

        pthread_mutex_lock(&bridge_mutex); // Bloquear acceso al puente

        if (strcmp(car->direccion, "der") == 0) {
            // Esperar si hay autos en el puente en dirección contraria, si hay autos esperando en dirección contraria
            // después de 4 consecutivos, o si el puente está lleno
            while ((on_bridge_left > 0) || 
                   (waiting_left_copy > 0 && consecutive_right == MAX_CONSECUTIVE) ||
                   (on_bridge_right == MAX_ON_BRIDGE)) {
                //printf("estoy esperando derecha soy %s\n", car->id);
                pthread_cond_wait(&turn_cond, &bridge_mutex);
            }
            // Permitir que el auto cruce el puente si hay autos esperando
            on_bridge_right++;
            consecutive_right++;
            waiting_right_copy--;
            consecutive_left = 0; // Reiniciar contador de autos consecutivos en dirección opuesta
            //printf("%s => %s cruzando\n", car->direccion, car->id);
            strcpy(cars_crossed_direction[cars_crossed++], "der");
    
        } else if (strcmp(car->direccion, "izq") == 0){
            // Esperar si hay autos en el puente en dirección contraria, si hay autos esperando en dirección contraria
            // después de 4 consecutivos, o si el puente está lleno
            while ((on_bridge_right > 0) ||
                   (waiting_right_copy > 0 && consecutive_left == MAX_CONSECUTIVE) ||
                   (on_bridge_left == MAX_ON_BRIDGE)) {
                //printf("estoy esperando izquierda soy %s\n", car->id);
                pthread_cond_wait(&turn_cond, &bridge_mutex);
            }
            // Permitir que el auto cruce el puente si hay autos esperando
                on_bridge_left++;
                consecutive_left++;
                waiting_left_copy--;
                consecutive_right = 0; // Reiniciar contador de autos consecutivos en dirección opuesta
                //printf("%s => %s cruzando\n", car->direccion, car->id);
                strcpy(cars_crossed_direction[cars_crossed++], "izq");   
            }
        
        pthread_mutex_unlock(&bridge_mutex); // Liberar acceso al puente
        sleep(3); // Simular tiempo de cruce del puente
        pthread_mutex_lock(&bridge_mutex); // Bloquear acceso al puente de nuevo

        if (strcmp(car->direccion, "der") == 0) {
            //printf("%s => %s se baja\n", car->direccion, car->id);
            on_bridge_right--;
        } else if (strcmp(car->direccion, "izq") == 0){
            //printf("%s => %s se baja\n", car->direccion, car->id);
            on_bridge_left--;
        }
        
        pthread_mutex_unlock(&bridge_mutex); // Liberar acceso al puente
        //printf("espacio libre\n");
        pthread_cond_broadcast(&turn_cond); // Notificar a otros autos que la condición ha cambiado
        free(car); // Liberar memoria del auto
        return;
    }


// Función para imprimir el estado actual del puente y las colas de espera
void print_status() {
    printf("** Autos en espera para atravesar el puente **\n");
    // Mostrar los autos en espera para cruzar
    for (int i = 0; i < car_count; i++) {
        if (strcmp(cars[i].direccion, "der") == 0) {
            printf("=> %s\n", cars[i].id);
        } else {
            printf("<= %s\n", cars[i].id);
        }
    }
    printf("> car _ _\n"); // Indicador de que se pueden agregar más autos
}

void print_cars(){
        printf("Array cars:\n");
    for (int i = 0; i < car_count; i++) {
        printf("Car ID: %s, Direccion: %s\n", cars[i].id, cars[i].direccion);
    }
}

void info(){
    print_cars();
    printf("cars_crossed_direction:\n");
    for (int i = 0; i < cars_crossed; i++) {
        printf("Direccion %d: %s\n", i, cars_crossed_direction[i]);
    }
}

void *command_handler(void *arg) {
    char command[20];

    while (1) {
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "car", 3) == 0) {
            if (strstr(command, "der") != NULL) {
                add_car("der");
            } else if (strstr(command, "izq") != NULL) {
                add_car("izq");
            }
        } else if (strcmp(command, "status") == 0) {
            system("clear");
            print_status();
        } else if (strcmp(command, "info") == 0) {
            info();
        }

    }
    return NULL;
}



void start(){
    pthread_t command_thread;
    pthread_create(&command_thread, NULL, command_handler, NULL);

    pthread_t threads[200];
    int thread_index = 0;
    
    while (1){
        //pthread_mutex_lock(&bridge_mutex);
        //while (waiting_right == 0 && waiting_left == 0) {
        //    pthread_cond_wait(&turn_cond, &bridge_mutex); // Esperar si no hay autos esperando
        //}
        //pthread_mutex_unlock(&bridge_mutex);

    while (waiting_right > 0 || waiting_left > 0) {
        if (waiting_right > 0) {
            Car *car = malloc(sizeof(Car));
            *car = cars_right[0];
            pthread_create(&threads[thread_index++], NULL, (void *)cross_bridge, (void *)car);
            memmove(cars_right, cars_right + 1, sizeof(Car) * (--waiting_right)); // Mover autos en la cola
        }
        if (waiting_left > 0) {
            Car *car = malloc(sizeof(Car));
            *car = cars_left[0];
            pthread_create(&threads[thread_index++], NULL, (void *)cross_bridge, (void *)car);
            memmove(cars_left, cars_left + 1, sizeof(Car) * (--waiting_left)); // Mover autos en la cola 
        }
    }

    
    sleep(3);

    strcpy(auto01, "      "); // Reinicia a cadena vacía
    strcpy(auto02, "      "); // Reinicia a cadena vacía
    strcpy(auto03, "      "); // Reinicia a cadena vacía


    for (int i = 0; i < cars_count_copy; i++) {                     // para cada auto
        if (strcmp(cars_crossed_direction[i], "der") == 0) {        // si su direccion es derecha
            for (int d = 0; d < car_count; d++) {                   // para cada auto
                if (strcmp(cars[d].direccion, "der") == 0) {        // busco en cars el primero que tenga derecha
                cars_printed_izq = 0;

                    // para el primer auto
                    if (cars_printed_der == 0) {    
                        system("clear");
                        strcpy(auto01, cars[d].id);
                        cars_printed_der++;
                        delete_car(cars, &car_count, d);
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>%s>>======>>      >>=======>>      >>====\n", auto01);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "der") != 0 || car_count == 0) {            //si el siguiente auto no le sigue
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>%s>>=======>>      >>====\n", auto02);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>%s>>====\n", auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>      >>====\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;

                    // para el segundo auto
                    } else if ( cars_printed_der == 1) {
                        system("clear");
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[d].id);
                        cars_printed_der++;
                        delete_car(cars, &car_count, d);
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>%s>>======>>%s>>=======>>      >>====\n", auto01, auto02);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "der") != 0 || car_count == 0) {                //si el siguiente auto no le sigue
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>%s>>=======>>%s>>====\n", auto02, auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>%s>>====\n", auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>      >>====\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;

                    // para el tercer auto
                    } else if (cars_printed_der == 2) {
                        system("clear");
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[d].id);
                        cars_printed_der++;
                        delete_car(cars, &car_count, d);
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>%s>>======>>%s>>=======>>%s>>====\n", auto01, auto02, auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "der") != 0 || car_count == 0) {                //si el siguiente auto no le sigue
                        printf("???\n");
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>%s>>=======>>%s>>====\n", auto02, auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>%s>>====\n", auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>      >>====\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;
                        
                    // para el cuarto auto
                    } else if (cars_printed_der == 3) {
                        system("clear");
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[d].id);
                        cars_printed_der++;
                        delete_car(cars, &car_count, d);
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>%s>>======>>%s>>=======>>%s>>====\n", auto01, auto02,auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        
                        while(car_count > 0 && strcmp(cars_crossed_direction[i+1], "der") == 0){
                            system("clear");
                            strcpy(auto03, auto02);
                            strcpy(auto02, auto01);
                            strcpy(auto01, cars[d].id);
                            delete_car(cars, &car_count, d);
                            printf("===================================================\n");
                            printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                            printf("====>>%s>>======>>%s>>=======>>%s>>====\n", auto01, auto02,auto03);
                            printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                            printf("===================================================\n");
                            print_status();
                            sleep(1);
                        }
                        
                        
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>%s>>=======>>%s>>====\n", auto02, auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>%s>>====\n", auto03);
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        
                        system("clear");
                        printf("===================================================\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("====>>      >>======>>      >>=======>>      >>====\n");
                        printf("====>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        break; 
                    }  
                }
            }

        } else if (strcmp(cars_crossed_direction[i], "izq") == 0) {    // si su direccion es izq
            for (int k = 0; k < car_count; k++) {                       // para cada auto
                if (strcmp(cars[k].direccion, "izq") == 0) {           //busco en cars el primero que tenga izq
                cars_printed_der = 0;
                    // para el primer auto
                    if (cars_printed_izq == 0) {    
                        system("clear");
                        strcpy(auto01, cars[k].id);
                        cars_printed_izq++;
                        delete_car(cars, &car_count, k);
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<      <<=======<<%s<<====\n", auto01);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "izq") != 0 || car_count == 0) {        //si el siguiente auto no le sigue
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<%s<<=======<<      <<====\n", auto02);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<      <<=======<<      <<====\n", auto03);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<      <<=======<<      <<====\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;

                    // para el segundo auto
                    } else if ( cars_printed_izq == 1) {
                        system("clear");
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[k].id);
                        cars_printed_izq++;
                        delete_car(cars, &car_count, k);
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<%s<<=======<<%s<<====\n",auto02, auto01);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "izq") != 0 || car_count == 0) {        //si el siguiente auto no le sigue
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<%s<<=======<<      <<====\n", auto03, auto02);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<      <<=======<<      <<====\n", auto03);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<      <<=======<<      <<====\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;

                    // para el tercer auto
                    } else if (cars_printed_izq == 2) {
                        system("clear");
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[k].id);
                        cars_printed_izq++;
                        delete_car(cars, &car_count, k);
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<%s<<=======<<%s<<====\n",auto03, auto02, auto01);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        if (strcmp(cars_crossed_direction[i+1], "izq") != 0 || car_count == 0) {        //si el siguiente auto no le sigue
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<%s<<=======<<      <<====\n", auto03, auto02);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<      <<=======<<      <<====\n", auto03);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<      <<=======<<      <<====\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        sleep(1);
                        }
                        break;

                    // para el cuarto auto
                    } else if (cars_printed_izq == 3) {
                        system("clear");
                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        strcpy(auto01, cars[k].id);
                        cars_printed_izq++;
                        delete_car(cars, &car_count, k);

                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<%s<<=======<<%s<<====\n", auto03, auto02, auto01);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        while(car_count > 0 && strcmp(cars_crossed_direction[i+1], "izq") == 0){
                            system("clear");
                            strcpy(auto03, auto02);
                            strcpy(auto02, auto01);
                            strcpy(auto01, cars[k].id);
                            delete_car(cars, &car_count, k);
                            printf("===================================================\n");
                            printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                            printf("====<<%s<<======<<%s<<=======<<%s<<====\n", auto03, auto02,auto01);
                            printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                            printf("===================================================\n");
                            print_status();
                            sleep(1);
                        }

                        strcpy(auto03, auto02);
                        strcpy(auto02, auto01);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<%s<<=======<<      <<====\n", auto03, auto02);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        strcpy(auto03, auto02);
                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<%s<<======<<      <<=======<<      <<====\n", auto03);
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);

                        system("clear");
                        printf("===================================================\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("====<<      <<======<<      <<=======<<      <<====\n");
                        printf("====<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<====\n");
                        printf("===================================================\n");
                        print_status();
                        sleep(1);
                        cars_printed_der == 0;
                        cars_printed_izq == 0;
                        break; 
                    }  
                }
            }
        }
    }
    printf("7 segundos para cargar mas autos\n");
    sleep(7);
    }
    for (int i = 0; i < thread_index; i++) {
        pthread_join(threads[i], NULL);
    }
}



// Función principal
int main() {
    char command[20];

    // Bucle para manejar los comandos del usuario
    while (1) {
        fgets(command, sizeof(command), stdin); // Leer comando del usuario
        command[strcspn(command, "\n")] = 0; // Eliminar el carácter de nueva línea

        if (strncmp(command, "car", 3) == 0) {
            // Agregar un auto en la dirección especificada
            if (strstr(command, "der") != NULL) {
                add_car("der");
            } else if (strstr(command, "izq") != NULL) {
                add_car("izq");
            }
        } else if (strcmp(command, "status") == 0) {
            system("clear");
            print_status(); // Imprimir el estado actual del puente y las colas de espera
        } else if (strcmp(command, "start") == 0) {
                start(); // Iniciar la simulación del puente
        }  
    }
    return 0;
}


