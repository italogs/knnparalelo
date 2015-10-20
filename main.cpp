#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include <mpi.h>
using namespace std;

void print_error(int error);
int getNAtributos(FILE *base);
bool compAsc(const pair<string, float> &a, const pair<string, float> &b);
bool compDesc(const pair<string, float> &a, const pair<string, float> &b);
void printVizinhos(map<string, float> vizinhos);
void printVector(vector<pair<string, float > > myvec,int mapSize);
vector<pair<string, float > > ordenaMap(map<string, float> map, int asc);
float distanciaEuclidiana(char *tuplaTreinamento,char *tuplaTeste,int nAtributos);
map<string, float> getKvizinhos(map<string, float> vizinhos, int k);
string predizerClasse(map<string, float> kvizinhos);
map<string, float> getVizinhos(FILE *baseTreinamento,char *tuplaTeste,int k,int nAtributos);


float calcularAcuracia(vector<string> predicoes,FILE *baseTeste,int nAtributos){
	int predicoes_corretas = 0;
	rewind(baseTeste);

	char *linhaTeste = NULL;
	size_t lenTeste = 0;
	int contador_Atributos = 0;
	int nLinhasBaseTeste=0;
	for (vector<string>::iterator it = predicoes.begin(); it != predicoes.end() ; ++it){
	
		getline(&linhaTeste, &lenTeste, baseTeste);
		nLinhasBaseTeste++;
		linhaTeste = strtok(linhaTeste,",");
		contador_Atributos = 1;
		while(contador_Atributos < nAtributos ){
			linhaTeste = strtok(NULL,",");
			contador_Atributos++;
		}
		if(strcmp(it->c_str(),linhaTeste) == 0) {
			predicoes_corretas++;
		} else {
			cout<<"Nao"<<endl;
		}

		linhaTeste = NULL;
	}
	return (predicoes_corretas/(nLinhasBaseTeste*1.0))*100.0;
}


int main(int argc,char *argv[]){

	int nthreads2, tid;

	int rank;
	char hostname[256];

	MPI_Init(&argc,&argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	gethostname(hostname,255);

	printf("Hello world!  I am process number: %d on host %s\n", rank, hostname);
		#pragma omp parallel private(nthreads2, tid)
		{
			tid = omp_get_thread_num();
			printf("Thread: %d\n",tid);
			if (tid == 0) {
				nthreads2 = omp_get_num_threads();
				printf("Number of threads = %d\n", nthreads2);
			}
		}
	cout<<"Espera aqui: "<<rank;

	
	if(rank == 0){
	
	
	int nthreads = 4, nmachines = 4, k = 3,option =0;
	float splitPoint = 66.6;
	char *filename = NULL;

	while((option = getopt(argc,argv,"t:k:n:f:s:")) != -1 ){

		switch(option){
			case 't': nthreads = atoi(optarg);
			break;

			case 'f': filename = optarg;
			break;

			case 'k': k = atoi(optarg);
			break;

			case 'n': nmachines = atoi(optarg);
			break;

			case 's': splitPoint = atof(optarg);
			break;

			default: print_error(1);
			break;
		}
	}
	// splitPoint = 0.67;
	cout<<"\n-----------\nEntrada:\nk: "<<k<<" , threads: "<<nthreads<<", machines: "<<nmachines<<", file: "<<filename<<", splitPoint: "<<splitPoint<<"\n-----------\n\n\n";

	char *linhaTreinamento = NULL;
	size_t lenTreinamento = 0;

	char *linhaTeste = NULL;
	size_t lenTeste = 0;

	FILE *baseOriginal = NULL;

	if(!filename)
		baseOriginal = fopen("iris.data","r");
	else
		baseOriginal = fopen(filename,"r");
	
	if(!baseOriginal) {
		cout<<"Arquivo nao existe.\n\n";
		exit(EXIT_FAILURE);
	}
	FILE *baseTreinamento = fopen("treinamento.data","w+");
	FILE *baseTeste = fopen("teste.data","w+");

	int nLinhasBaseOriginal = 0;
	while ((getline(&linhaTreinamento, &lenTreinamento, baseOriginal)) != -1)
       nLinhasBaseOriginal++;
   	
   	rewind(baseOriginal);

   	int splitBaseOriginal = (int) ((splitPoint * nLinhasBaseOriginal) / 100.0);
   	int nLinhasBaseTreinamento = 0;
   	int nLinhasBaseTeste = 0;
	while ((getline(&linhaTreinamento, &lenTreinamento, baseOriginal)) != -1) {
		if(nLinhasBaseTreinamento == splitBaseOriginal){
			fprintf(baseTeste,"%s",linhaTreinamento);
			nLinhasBaseTeste++;
		} else{
			fprintf(baseTreinamento,"%s",linhaTreinamento);
			nLinhasBaseTreinamento++;
		}
   	}
  
   	rewind(baseTreinamento);
   	int nAtributos = getNAtributos(baseTreinamento);
 	rewind(baseTreinamento);
   	rewind(baseTeste);
   	
  	vector<string> predicoes;
	while ((getline(&linhaTeste, &lenTeste, baseTeste)) != -1) {
	
		map<string, float> kVizinhos = getVizinhos(baseTreinamento,linhaTeste,k,nAtributos);
		predicoes.push_back(predizerClasse(kVizinhos));
		rewind(baseTreinamento);
	}

	float acuracia = calcularAcuracia(predicoes,baseTeste,nAtributos);
	cout<<"Acuracia do modelo: "<<acuracia<<endl;
	fclose(baseOriginal);
   	fclose(baseTreinamento);

   	fclose(baseTeste);

	if(linhaTreinamento)
	  free(linhaTreinamento);  
	if(linhaTeste)
		free(linhaTeste);


	}	
	MPI_Finalize();
	return 0;
}

void print_error(int error){
	switch(error){
		case 1:
			cout<<"Parametros incorretos";
		break;
		default:
			cout<<"Erro nao encontrado.";
		break;
	}
	cout<<endl<<endl;
	exit(EXIT_FAILURE);
}

int getNAtributos(FILE *base){
	char *linha = NULL;
	size_t len = 0;
	getline(&linha, &len, base);
	rewind(base);
	char *token = strtok(linha,",");
	int i = 1;
	while(token != NULL){
		token = strtok(NULL,",");
		i++;
	}
	return i-1;
}


void printVizinhos(map<string, float> vizinhos){
	cout<<"---------------\nprintVizinhos\n---------------\n";
	for (map<string, float>::iterator it=vizinhos.begin(); it!=vizinhos.end(); ++it)
   		cout << it->first << " => " << it->second<<endl;
}

void printVector(vector<pair<string, float > > myvec,int mapSize){
	cout<<"---------------\nprintVector\n---------------\n";
    for (int i = 0; i < mapSize; ++i)
        cout << i << ": " << myvec[i].first << "-> " << myvec[i].second << "\n";
}

bool compAsc(const pair<string, float> &a, const pair<string, float> &b) {
  return a.second < b.second;
}

bool compDesc(const pair<string, float> &a, const pair<string, float> &b) {
  return a.second > b.second;
}

vector<pair<string, float > > ordenaMap(map<string, float> map, int asc){

	vector<pair<string, float > > myvec(map.begin(), map.end());
	if(asc)
		sort(myvec.begin(),myvec.end(),compAsc);
	else
		sort(myvec.begin(),myvec.end(),compDesc);

	return myvec;
}

map<string, float> getKvizinhos(map<string, float> vizinhos, int k){

	vector<pair<string, float > > vector = ordenaMap(vizinhos,1);

	map<string, float> kvizinhos;

	for(int i = 0 ; i < k ; i++)
		kvizinhos.insert(kvizinhos.begin(),pair<string,float>(vector[i].first,0)); 

	return kvizinhos;
}

float distanciaEuclidiana(char *tuplaTreinamento,char *tuplaTeste,int nAtributos){
	float distancia = 0.0;
	char s[2] = ",";
	char *tokenTreinamento = NULL, *tokenTeste = NULL;
	char *buffTreinamento =  NULL, *buffTeste = NULL;
	tokenTreinamento = strtok_r(tuplaTreinamento,s,&buffTreinamento);
	tokenTeste = strtok_r(tuplaTeste,s,&buffTeste);
	int i = 1;
	
	while(tokenTreinamento != NULL && tokenTeste != NULL && i < nAtributos){
		distancia += pow(atof(tokenTreinamento) - atof(tokenTeste),2);
		tokenTreinamento = strtok_r(NULL,s,&buffTreinamento);
		tokenTeste = strtok_r(NULL,s,&buffTeste);
		i++;
	}
	tokenTreinamento = strtok_r(NULL,s,&buffTreinamento);
	tokenTeste = strtok_r(NULL,s,&buffTeste);
	return sqrt(distancia);
}


map<string, float> getVizinhos(FILE *baseTreinamento,char *tuplaTeste,int k,int nAtributos){
	char *tuplaTreinamento = NULL;
	size_t lenTreinamento = 0;
	ssize_t readTreinamento;
	float distancia=0.0;
	map<string, float> vizinhos;
	char *tuplaTesteAux = NULL, *tuplaTreinamentoAux = NULL;

	while ((readTreinamento = getline(&tuplaTreinamento, &lenTreinamento, baseTreinamento)) != -1) {
		tuplaTesteAux = (char *)malloc(sizeof(char)*strlen(tuplaTeste));
		strcpy(tuplaTesteAux,tuplaTeste);
		
		tuplaTreinamentoAux = (char *)malloc(sizeof(char)*lenTreinamento);
		strcpy(tuplaTreinamentoAux,tuplaTreinamento);

		distancia = distanciaEuclidiana(tuplaTreinamentoAux,tuplaTesteAux,nAtributos);
	
		string str(tuplaTreinamento);

		vizinhos.insert(vizinhos.begin(),pair<string,float>(str,distancia));
		free(tuplaTesteAux);
		free(tuplaTreinamentoAux);
		tuplaTesteAux = tuplaTreinamentoAux = NULL;
		
   	}

   	return getKvizinhos(vizinhos,k);
}

string predizerClasse(map<string, float> kvizinhos){
	map<string, float>::iterator it;
	char *linha2 = NULL;
	map<string, float> classes;
	for(it = kvizinhos.begin();it != kvizinhos.end();++it){

		string linha = it->first;

		linha2 = (char *)malloc(sizeof(char) * linha.size());
		strcpy(linha2,linha.c_str());

		char classeLinha[30];
		linha2 = strtok(linha2,",");
		while(linha2 !=NULL){
			strcpy(classeLinha,linha2);
			linha2 = strtok(NULL,",");
		}
		if(classes.find(classeLinha) != classes.end()){//elemento existe
			classes.at(classeLinha)++;
		} else {//elemento nao existe
			string temp(classeLinha);
			classes.insert(classes.begin(),pair<string, float >(temp,1));	
		}
		free(linha2);
		linha2 = NULL;
	}

	vector<pair<string, float > > vector_classes  = ordenaMap(classes,1);

	return vector_classes.begin()->first;
}
