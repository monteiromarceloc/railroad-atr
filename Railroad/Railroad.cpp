/*********************************************************************************
*
*  TRABALHO FINAL AUTOMAÇÃO EM TEMPO REAL - ELT012
*  CAMILA MARTINS E MARCELO MONTEIRO
*
**********************************************************************************/

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <process.h>						//_beginthreadex() e _endthreadex()
#include <conio.h>							//_getch
#include "CheckForError.h"
using namespace std;

#define TAM_LISTA 200
#define TAM_MSG 41

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	//Casting para terceiro e sexto parâmetros da função
													//_beginthreadex
typedef unsigned* CAST_LPDWORD;

#define NUM_THREADS_READ_REMOTE 2
#define	ESC	0x1B									// Tecla para encerrar o programa

DWORD WINAPI ThreadLeitura(int);
DWORD WINAPI ThreadRetirada();
//DWORD WINAPI ThreadPopMessage(int);
//DWORD WINAPI ThreadHotWheel(int);
//DWORD WINAPI ThreadShowData(int);
//DWORD WINAPI ThreadShowAlarms(int);
//DWORD WINAPI Thread(int);

HANDLE hMutex;							// Mutex base
HANDLE hSem;					        // Semáforo base
HANDLE hEventBlockRead;					// Evento de sinalização de bloqueio da tarefa de leitura
HANDLE hEventEnd;						// Evento de sinalização de término
HANDLE hEventTime;						// Evento para temporizadores tmeout (nunca será sinalizado)
HANDLE PosNova;
HANDLE hOut;							// Handle para a saída da console

char Mensagens[TAM_LISTA][TAM_MSG];					// Lista circular em memória
HANDLE hMutexNSEQ;									// Mutex para NSEQ
HANDLE hMutexPos;
int NSEQ, PosLivres, PosDepositar, PosRetirar;

SYSTEMTIME timestamp;

int nTecla;								//Variável que armazena a tecla digitada para sair

void gerarAlfaNumAleatorio(char* alfa, int len);

// THREAD PRIMÁRIA
int main() {
	HANDLE hThreadsRead[NUM_THREADS_READ_REMOTE];
	HANDLE hThreadPull;
	DWORD dwIdRR, dwIdPop, dwIdHW, dwIdSD, dwIdSA;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	NSEQ = 0;
	PosLivres = TAM_LISTA;
	PosDepositar = 0;
	PosRetirar = 0;

	int i;

	// --------------------------------------------------------------------------
	// Obtém um handle para a saída da console
	// --------------------------------------------------------------------------

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		SetConsoleTextAttribute(hOut, WHITE);
	printf("Erro ao obter handle para a saída da console\n");

	// --------------------------------------------------------------------------
	//  Cria objetos de sincronização
	// --------------------------------------------------------------------------

	hEventBlockRead = CreateEvent(NULL, TRUE, FALSE, "BlockEvento"); //Evento de 
	CheckForError(hEventBlockRead);
	hEventEnd = CreateEvent(NULL, TRUE, FALSE, "EndEvento"); //Evento de saída
	CheckForError(hEventEnd);
	hMutexNSEQ = CreateMutex(NULL, FALSE, "NumSequencial");
	CheckForError(hMutexNSEQ);
	PosNova = CreateEvent(NULL, FALSE, FALSE, "PosNova"); //Evento de nova posição
	CheckForError(PosNova);

	// --------------------------------------------------------------------------
	// Criação de threads
	// --------------------------------------------------------------------------

	// Threads Leitura
	for (i = 0; i < NUM_THREADS_READ_REMOTE; ++i) {
		hThreadsRead[i] = (HANDLE)_beginthreadex(
			NULL,
			0,
			(CAST_FUNCTION)ThreadLeitura,	//Casting necess�rio
			(LPVOID)i,
			0,
			(CAST_LPDWORD)&dwIdRR);		//Casting necess�rio
		if (hThreadsRead[i]) {
			SetConsoleTextAttribute(hOut, WHITE);
			printf("Thread leitura %d criado com Id=%0x\n", i, dwIdRR);
		}
		else {
			SetConsoleTextAttribute(hOut, WHITE);
			printf("Erro na criacao da thread leitura! N = %d Erro = %d\n", i, errno);
			exit(0);
		}
	}//for

	hThreadPull = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ThreadRetirada,	//Casting necess�rio
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdRR);


	// --------------------------------------------------------------------------
	// Leitura do teclado
	// --------------------------------------------------------------------------

	do {
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Tecle <Esc> para terminar\n");
		nTecla = _getch();
	} while (nTecla != ESC);

	// --------------------------------------------------------------------------
	// Aguarda término das threads e encerra programa
	// --------------------------------------------------------------------------

	dwRet = WaitForMultipleObjects(NUM_THREADS_READ_REMOTE, hThreadsRead, TRUE, INFINITE);
	CheckForError(dwRet == WAIT_OBJECT_0);

	// Fecha todos os handles de objetos do kernel
	for (int i = 0; i < NUM_THREADS_READ_REMOTE; ++i)
		CloseHandle(hThreadsRead[i]);
	//for

	CloseHandle(hMutexNSEQ);
	CloseHandle(hEventEnd);
	CloseHandle(hEventBlockRead);

	SetConsoleTextAttribute(hOut, WHITE);
	return EXIT_SUCCESS;

}//main

DWORD WINAPI ThreadLeitura(int i) {

	SetConsoleTextAttribute(hOut, HLRED);
	printf("Thread de leitura %d iniciando execucao...\n", i);

	char auxMensagem[TAM_MSG];
	DWORD status, ret;
	LONG dwContagemPrevia;
	int REMOTA, DIAG, ID_PARTE2, ESTADO, TIMESTAMP;
	char ID_PARTE1[3];

	GetLocalTime(&timestamp);

	do {

		// temporizador
		hEventTime = CreateEvent(NULL, TRUE, FALSE, "EvTimeOut");
		status = WaitForSingleObject(hEventTime, 500);
		if (status == WAIT_TIMEOUT) {

			// gerar mensagem
			WaitForSingleObject(hMutexNSEQ, INFINITE);
			NSEQ++;
			if (NSEQ == 1000000) NSEQ = 1;
			ReleaseMutex(hMutexNSEQ);

			REMOTA = (rand() % 2);
			DIAG = (rand() % 1000);
			gerarAlfaNumAleatorio(ID_PARTE1, 3);
			ID_PARTE2 = (rand() % 1000);
			ESTADO = (rand() % 2);
			sprintf(auxMensagem, "%07d;55;%02d;%03d;%s-%04d;%d;%02d:%02d:%02d", NSEQ, REMOTA, DIAG, ID_PARTE1, ID_PARTE2, ESTADO, timestamp.wHour, timestamp.wMinute, timestamp.wSecond);
			SetConsoleTextAttribute(hOut, HLRED);
			printf("Thread Leitura %d gerou a mensagem: %s.\n", i, auxMensagem);

			// verificar se há posição livre na lista
			WaitForSingleObject(hMutexPos, INFINITE);
			if (PosLivres <= 0) {
				SetConsoleTextAttribute(hOut, WHITE);
				printf("A lista circular em memoria esta cheia. Thread Leitura %d aguarda posição livre.\n", i);
				WaitForSingleObject(PosNova, INFINITE);
			}
			PosLivres--;
			strcpy(Mensagens[PosDepositar], auxMensagem);
			PosDepositar++;
			if (PosDepositar >= TAM_LISTA) {
				PosDepositar = 0;
			}
			ReleaseMutex(hMutexPos);

		}
		else {
			// não deve cair aqui.
			SetConsoleTextAttribute(hOut, HLRED);
			printf("Erro no temporizador da Thread Leitura %d.\n", i);
		}
	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLRED);
	printf("Thread Leitura %d encerrando execucao.\n", i);
	_endthreadex(0);
	return(0);
}//ThreadLeitura

DWORD WINAPI ThreadRetirada() {

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Thread de Retirada de Mensagens iniciando execucao...\n");

	char auxMensagem[TAM_MSG];
	DWORD status, ret;
	LONG dwContagemPrevia;

	do {
		SetConsoleTextAttribute(hOut, HLGREEN);
		printf("Thread de Retirada de Mensagens iniciando loop...\n");
		Sleep(200);
	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Thread de Retirada de Mensagens encerrando execucao.\n");
	_endthreadex(0);
	return(0);
}//ThreadLeitura

//função que gera sequencias alfanumericas aleatorias
void gerarAlfaNumAleatorio(char* alfa, int len) {
	char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < len; i++) {
		alfa[i] = charSet[rand() % (sizeof(charSet) - 1)];
	}
	alfa[len] = 0;
}
