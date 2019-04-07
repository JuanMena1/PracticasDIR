#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define D 3
#define NUM_NODOS (int)pow(2,D)

FILE *fp;

int size, rank;
MPI_Status status;

//Cuenta la cantidad de números que hay en el documento 'datos.dat'
int cantidadNumeros(){
	int n = 0;
	float num;
	fp = fopen("datos.dat", "r");
	while((fscanf(fp, "%f,", &num)) != EOF){
		n++;
	}
	return n;
}

//Obtiene los números del documento
void obtenerNumeros(float numeros[]){
	int i=0;
	float num;
	fp = fopen("datos.dat", "r");

	while((fscanf(fp, "%f,", &num)) != EOF){
		if(i<NUM_NODOS){
			numeros[i] = num;
			i++;
		}else 
			break;
	}
	fclose(fp);
}

//Envia los valores a cada proceso
void enviarDatos(float numeros[]){
	int i;
	for(i=0;i<NUM_NODOS-1;i++){
		MPI_Bsend(&numeros[i+1], 1, MPI_FLOAT, i+1, i, MPI_COMM_WORLD);
	}
}

//Obtiene los vecinos de cada nodo
void vecinosHipercubo(int vecinos[]){
	int i;
	for(i=0;i<D;i++){
		vecinos[i] = rank ^ (int)pow(2,i);
	}
}

//Calcula el mayor número de toda la red
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

	float numeros[NUM_NODOS];
	float mi_numero;
	int vecinos[D];
	int continua = 0;
	int numeros_documento = 0;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	numeros_documento = cantidadNumeros();

	if(size<NUM_NODOS){ //Comprueba que haya los suficientes procesos para ejecutar
		continua=1;
	} else if(numeros_documento < NUM_NODOS){ //Comprueba que haya los suficientes valores en 'datos.dat' para ejecutar
		continua=2;
	}

	if(rank==0){
		if(continua==1){ //Comprueba que haya los suficientes procesos para ejecutar
			fprintf(stderr, "ERROR, no se han lanzado los suficientes procesos, necesito al menos %d\n", NUM_NODOS);
		}
		else if (continua==2){ //Comprueba que haya los suficientes valores en 'datos.dat' para ejecutar
			fprintf(stderr, "ERROR, en el documento 'datos.dat' no hay suficientes valores, debe haber al menos %d\n", NUM_NODOS);
		} else {
			obtenerNumeros(&numeros);
			enviarDatos(&numeros);

			mi_numero=numeros[0]; // Se queda con el primer dato porque es necesario que intervenga en la red Hipercubo
			vecinosHipercubo(&vecinos);
			mi_numero = calcularMayor(mi_numero, &vecinos);
			printf("El máximo de la red HIPERCUBO es %2.2f\n",mi_numero);
		}
	}
	else {
		if(continua==0 && rank < NUM_NODOS){ //Comprueba que pueda ejecutar y que el proceso pertenezca a la red
			MPI_Recv(&mi_numero, 1, MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			vecinosHipercubo(&vecinos);
			mi_numero = calcularMayor(mi_numero, &vecinos);
		}
	}
	MPI_Finalize();
	return 0;
}