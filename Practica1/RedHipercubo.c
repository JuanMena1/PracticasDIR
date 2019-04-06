#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define D 3


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
	for(i=0;i<num_nodos-1;i++){
		MPI_Bsend(&numeros[i+1], 1, MPI_FLOAT, i+1, i, MPI_COMM_WORLD);
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

	if(rank==0){
		if(size<num_nodos){
			fprintf(stderr, "ERROR, no se han lanzado los suficientes procesos, necesito al menos %d\n", ((num_nodos)));
			exit(EXIT_FAILURE);
		}
		obtenerNumeros(&numeros);
		enviarDatos(&numeros);

		mi_numero=numeros[0]; // Se queda con el primer dato porque es necesario que intervenga en la red Hipercubo
		vecinosHipercubo(&vecinos);
		mi_numero = calcularMayor(mi_numero, &vecinos);
		printf("El máximo de la red HIPERCUBO es %2.2f\n",mi_numero);
	}
	else {
		if(rank<=num_nodos){
			MPI_Recv(&mi_numero, 1, MPI_FLOAT, MANEJADOR, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			vecinosHipercubo(&vecinos);
			mi_numero = calcularMayor(mi_numero, &vecinos);
		}
	}
	MPI_Finalize();
	return 0;
}