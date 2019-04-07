clean:
	rm RedToroide
	rm RedHipercubo

compileHipercubo:
	mpicc RedHipercubo.c -o RedHipercubo -lm

compileToroide:
	mpicc RedToroide.c -o RedToroide

runHipercubo:
	mpirun -n 8 RedHipercubo

runToroide:
	mpirun -n 10 RedToroide