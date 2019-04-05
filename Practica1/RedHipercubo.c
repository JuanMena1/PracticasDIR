#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define MANEJADOR size-1
#define D 4


int num_nodos = pow(2,D);
FILE *fp;

int size, rank;
MPI_Status status;

void obtenerNumeros(float numeros[]){
	int i=0;
	float num;
	fp = fopen("datos.dat", "r");

	while((fscanf(fp, "%f,", &num)) != EOF){
		if(i<num_nodos){
			numeros[i] = num;
			i++;
		}else 
			break;
	}
	fclose(fp);
}

void enviarDatos(float numeros[]){
	int i;
	for(i=0;i<num_nodos;i++){
		MPI_Bsend(&numeros[i], 1, MPI_FLOAT, i, i, MPI_COMM_WORLD);
	}
}

void vecinosHipercubo(int vecinos[]){
	int i;
	for(i=0;i<D;i++){
		vecinos[i] = rank ^ (int)pow(2,i);
	}
}

float calcularMayor(float mi_numero, int vecinos[]){
	int i;
	float su_numero;

	//Envio por dimensiones
		for(i=1;i<=D;i++){
			MPI_Bsend(&mi_numero, 1, MPI_FLOAT, vecinos[i-1], 0, MPI_COMM_WORLD);
			MPI_Recv(&su_numero, 1, MPI_FLOAT, vecinos[i-1], 0, MPI_COMM_WORLD, &status);
			mi_numero = MAX(mi_numero, su_numero);
		}
		return mi_numero;
}

int main(int argc, char *argv[])
{

	float numeros[num_nodos];
	float mi_numero;
	int vecinos[D];

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int a;

	if(rank==MANEJADOR){
		if(size<num_nodos+1){
			fprintf(stderr, "ERROR, no se han lanzado los suficientes procesos, necesito al menos %d\n", ((num_nodos)+1));
			exit(EXIT_FAILURE);
		}
		obtenerNumeros(&numeros);
		enviarDatos(&numeros);
		MPI_Recv(&mi_numero, 1, MPI_FLOAT, 1, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
		printf("El máximo de la red HIPERCUBO es %2.2f\n",mi_numero);
	}
	else {
		MPI_Recv(&mi_numero, 1, MPI_FLOAT, MANEJADOR, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
		vecinosHipercubo(&vecinos);
		mi_numero = calcularMayor(mi_numero, &vecinos);
		if(rank==1)
			MPI_Bsend(&mi_numero, 1, MPI_FLOAT, MANEJADOR, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}