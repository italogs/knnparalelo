run: compile 
	mpirun -np 4 exec.o 

compile: 
	mpic++ -lm -W  -pedantic -fopenmp main.cpp -o exec.o 

# delete:
	# rm main teste.data treinamento.data