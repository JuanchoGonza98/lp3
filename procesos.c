#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <string.h>

/*Funcion para ordenamiento auxiliar para ordenamiento de arrays correspondiente
a cada proceso*/
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
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}
void printProcessData(int processId, int depth, int data[], int size) {
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("Proceso %d: ", processId);
    printArray(data, size);
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
/*Calculoo para trazados de ramas del arbol*/
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

    printSpaces(spacesBeforeNodes / 2); // Adjust for the first node

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
/*Se busca una especie de estructura para los procesos Padre-Hijo*/
void processSort(int data[], int size, int depth, int maxDepth, int processId) {
    if (depth > maxDepth) {
        return; // If beyond max depth, return
    }
 if (depth == maxDepth) {
        exit(0); // Terminate the child process
    }

    int mid = size / 2;
    pid_t pidLeft = fork();
    if (pidLeft == 0) {
        processSort(data, mid, depth + 1, maxDepth, processId * 2 + 1);
    } else {
        waitpid(pidLeft, NULL, 0); // The parent process waits for the left child
        pid_t pidRight = fork();
        if (pidRight == 0) {
            processSort(data + mid, size - mid, depth + 1, maxDepth, processId * 2 + 2);
        } else {
            waitpid(pidRight, NULL, 0); // The parent process waits for the right child
        }
    }
}
//===============Funcion para realizar el merge de arrays ==============
void processSortmerge(int data[], int size, int depth, int processId, int maxDepth) {
    // Base case: if the current process is at the maximum depth, it is a leaf.
    if (depth == maxDepth) {
        bubbleSort(data, size);  // Sort the leaf node data.
        printf("proceso %d lista ordenada: {", processId);
        printArray(data, size);
        printf("}\n");
        return;
    }

    // Split the data into two halves.
    int mid = size / 2;
    int leftData[mid], rightData[size - mid];
    memcpy(leftData, data, mid * sizeof(int));
    memcpy(rightData, data + mid, (size - mid) * sizeof(int));

    // Fork two child processes for left and right halves of the data.
    pid_t pidLeft, pidRight;

    pidLeft = fork();
    if (pidLeft == 0) {  // Child process takes over the left half.
        processSortmerge(leftData, mid, depth + 1, 2 * processId + 1, maxDepth);
        exit(0);
    }

    pidRight = fork();
    if (pidRight == 0) {  // Child process takes over the right half.
        processSortmerge(rightData, size - mid, depth + 1, 2 * processId + 2, maxDepth);
        exit(0);
    }

    // Wait for both children to finish their sorting tasks.
    waitpid(pidLeft, NULL, 0);
    waitpid(pidRight, NULL, 0);

    // After both children have sorted their parts, merge the results.
    merge(data, leftData, rightData, mid, size - mid);
    // Print the merged data along with child data for verification.
    printf("proceso %d: lista izquierda {", 2 * processId + 1);
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

    if (fork() == 0) {
        processSort(arr, i, 0, maxDepth, 0);
        exit(0);
    } else {
        wait(NULL); // Wait for the root process to finish
        printf("\n====esquema de arbol ====\n");
        printTreeStructure(0, maxDepth);
        printf("\n====fin de esquema de arbol====\n");
        printf("\n====mapeo====\n");
        processSortmap(arr, i, 0, 0,maxDepth);
        printf("\n====fin del mapeo====\n");
        printf("==========procesamiento=======\n");
        processSortmerge(arr, i, 0, 0, maxDepth);
    }
    return 0;
}