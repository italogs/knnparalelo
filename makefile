run: compile 
	./exec -t 4 -n 4 -k 3 -f "iris.data" -s 66.6 

compile: 
	g++ -o exec main.cpp -lm -Wall -Werror -pedantic 

# delete:
	# rm main teste.data treinamento.data