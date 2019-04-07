#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define MANEJADOR 0
#define L 3
#define NUM_NODOS L*L

#define NORTE 0
#define SUR 1
#define ESTE 2
#define OESTE 3


float numeros[NUM_NODOS];
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
void obtenerNumeros(){
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
void enviarDatos(){
	int i;
	for(i=0;i<NUM_NODOS;i++){
		MPI_Bsend(&numeros[i], 1, MPI_FLOAT, i+1, i, MPI_COMM_WORLD);
	}
}

//Obtiene los vecinos de cada nodo
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

//Calcula el menor número de toda la red
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

	float mi_numero;
	int vecinos[4];
	int continua=0;
	int numeros_documento = 0;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	numeros_documento = cantidadNumeros();

	if(size<(NUM_NODOS)+1){ //Comprueba que haya los suficientes procesos para ejecutar
		continua=1;
	} else if(numeros_documento < NUM_NODOS){ //Comprueba que haya los suficientes valores en 'datos.dat' para ejecutar
		continua=2;
	}


	if(rank==MANEJADOR){
		if(continua==1){ //Comprueba que haya los suficientes procesos para ejecutar
			fprintf(stderr, "ERROR, no se han lanzado los suficientes procesos, necesito al menos %d\n", (NUM_NODOS)+1);
		}
		else if (continua==2){ //Comprueba que haya los suficientes valores en 'datos.dat' para ejecutar
			fprintf(stderr, "ERROR, en el documento 'datos.dat' no hay suficientes valores, debe haber al menos %d\n", NUM_NODOS);
		} else {
			obtenerNumeros();
			enviarDatos();
			MPI_Recv(&mi_numero, 1, MPI_FLOAT, 1, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			printf("El mínimo de la red TOROIDE es %2.2f\n",mi_numero);
		}
	}
	else {
		if(continua==0 && rank < (NUM_NODOS)+1){ //Comprueba que pueda ejecutar y que el proceso pertenezca a la red
			MPI_Recv(&mi_numero, 1, MPI_FLOAT, MANEJADOR, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			vecinosToroide(&vecinos);
			mi_numero = calcularMenor(mi_numero, &vecinos);
			if(rank==1)
				MPI_Bsend(&mi_numero, 1, MPI_FLOAT, MANEJADOR, 0, MPI_COMM_WORLD);
		}
	}
	
	MPI_Finalize();
	return 0;
}