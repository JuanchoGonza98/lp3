#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <string.h>


/*Funcion para ordenamiento auxiliar para ordenamiento de arrays correspondiente a cada proceso*/
void bubbleSort(int arr[], int n) {
    int i, j, temp;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

/*Algoritmo de merge*/
void merge(int data[], int leftData[], int rightData[], int leftSize, int rightSize) {
    int i = 0, j = 0, k = 0;

    while (i < leftSize && j < rightSize) {
        if (leftData[i] <= rightData[j]) {
            data[k++] = leftData[i++];
        } else {
            data[k++] = rightData[j++];
        }
    }

    while (i < leftSize) {
        data[k++] = leftData[i++];
    }

    while (j < rightSize) {
        data[k++] = rightData[j++];
    }
}


/*Funcion para imprimir arrays*/
void printArray(int arr[], int size) {
    for (int i = 0; i < size-1; i++)
        printf("%d,", arr[i]);

    printf("%d", arr[size-1]);
}

void printProcessData(int processId, int depth, int data[], int size) {
    printf("Proceso %d: ", processId);
    printArray(data, size);
    printf("\n");
}  

/*Metodo para mapeo*/
void processSortmap(int data[], int size, int depth, int processId, int maxDepth) {
    printProcessData(processId, depth, data, size);

    if (depth >= maxDepth) {
        return;
    }

    int mid = size / 2;

    pid_t pidLeft = fork();
    if (pidLeft == 0) {
        processSortmap(data, mid, depth + 1, 2 * processId + 1, maxDepth);
        exit(0);
    }

    pid_t pidRight = fork();
    if (pidRight == 0) {
        processSortmap(data + mid, size - mid, depth + 1, 2 * processId + 2, maxDepth);
        exit(0);
    }

    waitpid(pidLeft, NULL, 0);
    waitpid(pidRight, NULL, 0);
}

/*Calculo de profundidad del arbol*/
int calculateMaxDepth(int numberOfProcesses) {
    return ceil(log2(numberOfProcesses + 1)) - 1;
}


/*Calculo para espaciado de graficos*/
void printSpaces(int count) {
    for (int i = 0; i < count; ++i) {
        printf(" ");
    }
}
/*Calculo para trazados de ramas del arbol*/
void printBranches(int branchLevel, int maxDepth) {
    if (branchLevel > maxDepth) return;

    int spacesBetweenBranches = (1 << (maxDepth - branchLevel + 1)) - 1;
    int spacesBeforeBranches = (1 << (maxDepth - branchLevel)) - 1;
    int numBranches = (1 << branchLevel);

    for (int i = 0; i < numBranches; ++i) {
        printSpaces(spacesBeforeBranches);
        printf("/");
        printSpaces(spacesBetweenBranches);
        printf("\\");
        printSpaces(spacesBeforeBranches);

        if (i < numBranches - 1) {
            printSpaces(spacesBetweenBranches);
        }
    }
    printf("\n");
}
/*Funcion para dibujar los nodos*/
void printNodes(int nodeLevel, int maxDepth) {
    if (nodeLevel > maxDepth) return;

    int spacesBetweenNodes = (1 << (maxDepth - nodeLevel + 2)) - 1;
    int spacesBeforeNodes = (1 << (maxDepth - nodeLevel + 1)) - 1;
    int numNodes = (1 << nodeLevel);

    printSpaces(spacesBeforeNodes / 2); 

    for (int i = 0; i < numNodes; ++i) {
        printf("(%d)", i + (1 << nodeLevel) - 1);

        if (i < numNodes - 1) {
            printSpaces(spacesBetweenNodes);
        }
    }
    printf("\n");
}
/*FUNCION PARA IMPRIMIR EL GRAFICO DE ARBOL*/
void printTreeStructure(int currentDepth, int maxDepth) {
    if (currentDepth > maxDepth) return;

    printNodes(currentDepth, maxDepth);

    if (currentDepth < maxDepth) {
        printBranches(currentDepth, maxDepth);
        printTreeStructure(currentDepth + 1, maxDepth);
    }
}


void printTree(int process_id, int parent_id, const char* relation) {
    if(parent_id == -1)
        printf("proceso %d (%s): ", process_id, relation);
    else{
    printf("proceso %d (%s de proceso %d): ", process_id, relation, parent_id);
    }
}

void processSort(int arr[], int size, int depth, int process_id, int parent_id) {
    // Si el parent_id es -1, entonces es el origen, de lo contrario, es un hijo.
    // Si el process_id es igual a 2 * parent_id + 1, entonces es un hijo izquierdo, de lo contrario, es un hijo derecho.
    const char* relation = parent_id == -1 ? "Origen" : (process_id == 2 * parent_id + 1 ? "izq" : "der");

    printTree(process_id, parent_id, relation);
    printArray(arr, size);
    printf("\n");

    if (depth > 0) {
        int mid = size / 2;

        int pid = fork();       // Hijo izquierdo
        if (pid == 0) { 
            processSort(arr, mid, depth - 1, 2 * process_id + 1, process_id);
            exit(0);
        }

        int pid2 = fork();      // Hijo derecho
        if (pid2 == 0) { 
            processSort(arr + mid, size - mid, depth - 1, 2 * process_id + 2, process_id);
            exit(0);
        }

        wait(NULL);
        wait(NULL);
    }
}



/*===============Funcion para realizar el merge de arrays ==============*/
void processSortmerge(int data[], int size, int depth, int processId, int maxDepth) {
    // Caso base: si la profundidad es igual a la profundidad máxima, es una hoja.
    if (depth == maxDepth) {
        bubbleSort(data, size);  // Ordenar el array.
        printf("proceso %d lista ordenada: {", processId);
        printArray(data, size);
        printf("}\n");
        return;
    }

    // Dividir el array en dos mitades.
    int mid = size / 2;
    int leftData[mid], rightData[size - mid];
    memcpy(leftData, data, mid * sizeof(int));
    memcpy(rightData, data + mid, (size - mid) * sizeof(int));

    // Crear cuatro pipes para comunicar los datos entre los procesos.
    pid_t pidLeft, pidRight;
    int pipeLeft1[2], pipeRight1[2];
    int pipeLeft2[2], pipeRight2[2];

    pipe(pipeLeft1);
    pipe(pipeRight1);
    pipe(pipeLeft2);
    pipe(pipeRight2);

    // Crear dos procesos hijos para ordenar las dos mitades del array.
    // Se crea el proceso izquierdo.
    pidLeft = fork();
    if (pidLeft == 0) {              
        close(pipeLeft1[1]);
        close(pipeLeft2[0]);
        read(pipeLeft1[0], leftData, mid * sizeof(int));       
        processSortmerge(leftData, mid, depth + 1, 2 * processId + 1, maxDepth);
        write(pipeLeft2[1], leftData, mid * sizeof(int));
        close(pipeLeft2[1]);
        exit(0);
    }
    
    close(pipeLeft1[0]);
    close(pipeLeft2[1]);
    write(pipeLeft1[1], leftData, mid * sizeof(int));
    close(pipeLeft1[1]);
    read(pipeLeft2[0], leftData, mid * sizeof(int));
    wait(NULL);
    close(pipeLeft2[0]);

    // Se crea el proceso derecho.
    pidRight = fork();
    if (pidRight == 0) {                
        close(pipeRight1[1]);
        close(pipeRight2[0]);
        read(pipeRight1[0], rightData, (size - mid) * sizeof(int));                   
        processSortmerge(rightData, size - mid, depth + 1, 2 * processId + 2, maxDepth);
        write(pipeRight2[1], rightData, (size - mid) * sizeof(int));
        close(pipeRight2[1]);
        exit(0);
    }

    close(pipeRight1[0]);
    close(pipeRight2[1]);
    write(pipeRight1[1], rightData, (size - mid) * sizeof(int));
    close(pipeRight1[1]);
    read(pipeRight2[0], rightData, (size - mid) * sizeof(int));
    wait(NULL);
    close(pipeRight2[0]);

    // Combinar los datos ordenados de los dos procesos hijos.
    merge(data, leftData, rightData, mid, size - mid);

    // Imprimir los datos de los procesos.
    printf("proceso %d: lista izquierda {",processId);
    printArray(leftData, mid);
    printf("}, lista derecha {");
    printArray(rightData, size - mid);
    printf("} => ");
    printArray(data, size);
    printf("\n");
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_processes> <array_of_integers>\n", argv[0]);
        return 1;
    }

    int num_processes = atoi(argv[1]);
    int maxDepth = calculateMaxDepth(num_processes);

    int arr[1000], i = 0;
    char *token = strtok(argv[2], ",");
    while (token != NULL) {
        arr[i++] = atoi(token);
        token = strtok(NULL, ",");
    }


    printf("\n====esquema de arbol ====\n");
    printTreeStructure(0, maxDepth);
    processSort(arr, i, maxDepth, 0, -1);
    printf("====fin de esquema de arbol====\n");

    printf("\n====mapeo====\n");
    processSortmap(arr, i, 0, 0,maxDepth);
    printf("====fin del mapeo====\n");

    printf("\n==========procesamiento=======\n");
    processSortmerge(arr, i, 0, 0, maxDepth);



    return 0;
}