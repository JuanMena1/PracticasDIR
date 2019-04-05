#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define MANEJADOR 0
#define L 4

#define NORTE 0
#define SUR 1
#define ESTE 2
#define OESTE 3

float numeros[L*L];
FILE *fp;

int size, rank;
MPI_Status status;

void obtenerNumeros(){
	int i=0;
	float num;
	fp = fopen("datos.dat", "r");

	while((fscanf(fp, "%f,", &num)) != EOF){
		if(i<L*L){
			numeros[i] = num;
			i++;
		}else 
			break;
	}
	fclose(fp);
}

void enviarDatos(){
	int i;
	for(i=0;i<L*L;i++){
		MPI_Bsend(&numeros[i], 1, MPI_FLOAT, i+1, i, MPI_COMM_WORLD);
	}
}

void vecinosToroide(int vecinos[]){
	int nodo = rank;
	int fila = (nodo-1)/L;
	int columna = (nodo-1)%L;

	//Calculamos Sur en caso de que nos toque la fila 0
	if (fila == 0){
		vecinos[SUR] = nodo +((L-1)*L);
	}else{
		vecinos[SUR] = nodo-L;
	}

	//Calculamos Norte en caso que nos toque la fila más arriba
	if (fila == L-1){
		vecinos[NORTE] = nodo-(fila*L);
	}else{
		vecinos[NORTE] = nodo+L;
	}
	//Calculamos Oeste en caso de que la columna sea 0
	if (columna == 0){
		vecinos[OESTE] = nodo+(L-1);
	}else{
		vecinos[OESTE] = nodo-1;
	}
	//Calculamos el Este en caso de que sea la columna de más a la derecha
	if (columna == L-1) {
		vecinos[ESTE] = nodo-(L-1);
	}else{
		vecinos[ESTE] = nodo+1;
	}
}

float calcularMenor(float mi_numero, int vecinos[]){
	int i;
	float su_numero;

	//Envio vertical
		for(i=1;i<L;i++){
			MPI_Bsend(&mi_numero, 1, MPI_FLOAT, vecinos[SUR], i, MPI_COMM_WORLD);
			MPI_Recv(&su_numero, 1, MPI_FLOAT, vecinos[NORTE], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			mi_numero = MIN(mi_numero, su_numero);
		}
		//Envio horizontal
		for(i=1;i<L;i++){
			MPI_Bsend(&mi_numero, 1, MPI_FLOAT, vecinos[ESTE], i, MPI_COMM_WORLD);
			MPI_Recv(&su_numero, 1, MPI_FLOAT, vecinos[OESTE], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			mi_numero = MIN(mi_numero, su_numero);
		}
		return mi_numero;
}

int main(int argc, char *argv[])
{

	//float minimo;
	float mi_numero;
	int vecinos[4];

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	if(rank==MANEJADOR){
		if(size<(L*L)+1){
			fprintf(stderr, "ERROR, no se han lanzado los suficientes procesos, necesito al menos %d\n", ((L*L)+1));
			exit(EXIT_FAILURE);
		}
		obtenerNumeros();
		enviarDatos();
		MPI_Recv(&mi_numero, 1, MPI_FLOAT, 1, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
		printf("El mínimo de la red TOROIDE es %2.2f\n",mi_numero);
	}
	else {
		MPI_Recv(&mi_numero, 1, MPI_FLOAT, MANEJADOR, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
		vecinosToroide(&vecinos);
		mi_numero = calcularMenor(mi_numero, &vecinos);
		if(rank==1)
			MPI_Bsend(&mi_numero, 1, MPI_FLOAT, MANEJADOR, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}