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
#include <process.h>						//_beginthreadex() e _endthreadex()
#include <conio.h>							//_getch
#include "CheckForError.h"

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	//Casting para terceiro e sexto parâmetros da função
													//_beginthreadex
typedef unsigned* CAST_LPDWORD;

#define NUM_THREADS_READ_REMOTE 2
#define	ESC				0x1B			// Tecla para encerrar o programa

DWORD WINAPI ThreadReadRemote(int);
DWORD WINAPI ThreadPopMessage(int);
DWORD WINAPI ThreadHotWheel(int);
DWORD WINAPI ThreadShowData(int);
DWORD WINAPI ThreadShowAlarms(int);
DWORD WINAPI Thread(int);

HANDLE hMutex;							// Mutex base
HANDLE hSem;					        // Semáforo base
HANDLE hEventBlockRead;					// Evento de sinalização de bloquei da tarefa de leitura
HANDLE hEventEnd;						// Evento de sinalização de término
HANDLE hOut;							// Handle para a saída da console

int nTecla;								//Variável que armazena a tecla digitada para sair

// THREAD PRIMÁRIA
int main() {
	HANDLE hThreadsRead[NUM_THREADS_READ_REMOTE];
	HANDLE hThreadPop, hThreadHotWheel, hThreadShowData, hThreadShowAlarms;
	DWORD dwIdRR, dwIdPop, dwIdHW, dwIdSD, dwIdSA;
	DWORD dwExitCode = 0;
	DWORD dwRet;
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

	hEventBlockRead = CreateEvent(NULL, TRUE, FALSE, "EscEvento"); //Evento de reset manual
	CheckForError(hEventBlockRead);
	hEventEnd = CreateEvent(NULL, TRUE, FALSE, "EscEvento"); //Evento de reset manual
	CheckForError(hEventEnd);

	// --------------------------------------------------------------------------
	// Criação de threads
	// --------------------------------------------------------------------------

	// Threads Leitura
	for (i = 0; i < NUM_THREADS_READ_REMOTE; ++i) {
		hThreadsRead[i] = (HANDLE)_beginthreadex(
			NULL,
			0,
			(CAST_FUNCTION)ThreadReadRemote,	//Casting necess�rio
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

	SetConsoleTextAttribute(hOut, WHITE);
	return EXIT_SUCCESS;

}//main

DWORD WINAPI ThreadReadRemote(int i) {

	DWORD status;
	LONG dwContagemPrevia;

	do {
		// Acessa contador de porcoes
		SetConsoleTextAttribute(hOut, HLRED);
		printf("Thread de leitura %d iniciando execucao...\n", i);

		Sleep(rand() % 5 * 200);
	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLRED);
	printf("Thread Leitura %d encerrando execucao.\n", i);
	_endthreadex(0);
	return(0);
}//ThreadFilhote

//função que gera numeros inteiros aleatorios
int gerarNumeroAleatorioInteiro(int digit) {
	int y = pow(10, digit);
	int x = rand() % y;
	return x;
}

//função que gera numeros reais aleatorios 
float gerarNumeroAleatorioReal(int digit) {
	float y = pow(10.0, digit - 2);
	float x = fmod(rand(), (y));
	float z = (rand() % 10) / 10.0;
	x = x + z;
	return x;
}

//função que gera sequencias alfanumericas aleatorias
void gerarAlfaNumAleatorio(char* alfa, int len) {
	char charSet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < len; i++) {
		alfa[i] = charSet[rand() % (sizeof(charSet) - 1)];
	}
	alfa[len] = 0;
}