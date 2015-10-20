run: compile 
	mpirun -np 4 exec.o 

compile: 
	mpic++ -lm -W -Wall -ansi  -Wno-long-long -pedantic -Werror -fopenmp main.cpp -o exec.o 

# delete:
	# rm main teste.data treinamento.data