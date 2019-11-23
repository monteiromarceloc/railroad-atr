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
using namespace std;

#define TAM_LISTA 200
#define TAM_ARQUIVO 500
#define TAM_MSG 41
#define NUM_THREADS_LEITURA 2
#define	ESC	0x1B

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	//Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned* CAST_LPDWORD;

DWORD WINAPI ThreadDetectora();
DWORD WINAPI ThreadRetirada();

HANDLE hOut;							// Handle para a saída da console
HANDLE hEventEnd;						// Evento de sinalização de término
HANDLE hEventRetirada;					// Evento para ativar e desativar a thread retirada 
HANDLE hEventDetect;					// Evento para ativar e desativar a thread leitura 
HANDLE hEventTime;						// Evento para temporizadores timeout (nunca será sinalizado)
HANDLE PosNova;							// Evento para posição liberada
HANDLE hMutexNSEQ2;						// Mutex para NSEQ2
HANDLE hMutexPos;						// Mutex para variáveis de posição

int NSEQ2, PosLivres, PosDepositar, PosRetirar, PosLivresArquivo;
bool EnableDetect;

char Mensagens[TAM_LISTA][TAM_MSG];					// Lista circular em memória
SYSTEMTIME timestamp;
int nTecla;								//Variável que armazena a tecla digitada para sair
void gerarAlfaNumAleatorio(char* alfa, int len);

// THREAD PRIMÁRIA
int main() {

	HANDLE hThreadDetec;
	HANDLE Events[2] = { hEventDetect, hEventEnd };
	DWORD dwIdRR, dwIdPop, dwIdHW, dwIdSD, dwIdSA;
	DWORD dwExitCode = 0;
	DWORD dwRet, ret;
	int nTipoEvento;
	NSEQ2 = 0;
	PosLivres = TAM_LISTA;
	PosDepositar = 0;
	PosRetirar = 0;
	EnableDetect = TRUE;

	int i;

	// --------------------------------------------------------------------------
	// Obtém um handle para a saída da console
	// --------------------------------------------------------------------------

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) {
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Erro ao obter handle para a saída da console\n");
	}
	// --------------------------------------------------------------------------
	//  Cria objetos de sincronização
	// --------------------------------------------------------------------------

	// TODO: Mudar para OpenEvent depois que ele for criado pelo teclado
	hEventEnd = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EndEvento"); //Evento de encerramento da thread de leitura
	hEventDetect = OpenEvent(EVENT_ALL_ACCESS, FALSE, "LeituraDetectoresON-OFF"); // Evento toggle da thread de Leitura
	hMutexNSEQ2 = CreateMutex(NULL, FALSE, "NumSequencial");
	//PosNova = CreateEvent(NULL, FALSE, FALSE, "PosNova"); //Evento de nova posição


	// --------------------------------------------------------------------------
	// Criação de threads
	// --------------------------------------------------------------------------

	// Threads Leitura
	hThreadDetec = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ThreadDetectora,	//Casting necess�rio
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdRR);


	// --------------------------------------------------------------------------
	// Leitura do teclado
	// --------------------------------------------------------------------------

	SetConsoleTextAttribute(hOut, WHITE);
	printf("Tecle <Esc> para terminar\n");

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hEventDetect);
			SetConsoleTextAttribute(hOut, WHITE);
			printf("Threads Detectora: OFF\n");
			EnableDetect = FALSE;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				ResetEvent(hEventDetect);
				SetConsoleTextAttribute(hOut, WHITE);
				printf("Threads Detectora: ON\n");
				EnableDetect = TRUE;
			}
			else if (nTipoEvento == 1) {
				break;
			}
		}
		else if (nTipoEvento == 1) { break; }
	} while (nTipoEvento != 1);

	// --------------------------------------------------------------------------
	// Aguarda término das threads e encerra programa
	// --------------------------------------------------------------------------

	dwRet = WaitForSingleObject(hThreadDetec, INFINITE);

	CloseHandle(hThreadDetec);
	CloseHandle(hEventEnd);
	CloseHandle(hEventDetect);
	CloseHandle(hEventRetirada);
	CloseHandle(hMutexNSEQ2);
	CloseHandle(PosNova);

	SetConsoleTextAttribute(hOut, WHITE);
	return EXIT_SUCCESS;

}//main

DWORD WINAPI ThreadDetectora() {

	SetConsoleTextAttribute(hOut, HLRED);
	printf("Thread Detectora iniciando execucao...\n");

	char auxMensagem[TAM_MSG];
	DWORD status, ret;
	LONG dwContagemPrevia;
	int REMOTA, DIAG, ID_PARTE2, ESTADO, TIMESTAMP;
	char ID_PARTE1[3];

	do {
		// temporizador
		hEventTime = CreateEvent(NULL, TRUE, FALSE, "EvTimeOut");
		status = WaitForSingleObject(hEventTime, 2000);

		if (EnableDetect) {
			if (status == WAIT_TIMEOUT) {
				// GERAR MENSAGEM
				WaitForSingleObject(hMutexNSEQ2, INFINITE);
				NSEQ2++;
				if (NSEQ2 == 1000000) NSEQ2 = 1;
				GetLocalTime(&timestamp);
				gerarAlfaNumAleatorio(ID_PARTE1, 3);
				ID_PARTE2 = (rand() % 1000);
				ESTADO = (rand() % 4 == 3 ? 1 : 0);
				sprintf_s(auxMensagem, "%07d;00;%s-%04d;%d;%02d:%02d:%02d", NSEQ2, ID_PARTE1, ID_PARTE2, ESTADO, timestamp.wHour, timestamp.wMinute, timestamp.wSecond);
				SetConsoleTextAttribute(hOut, HLRED);
				printf("Thread Detectora gerou a mensagem: %s.\n", auxMensagem);
				ReleaseMutex(hMutexNSEQ2);

				// TO DO ENVIAR PARA O EXIBIÇÃO
			}
			else {
				// não deve cair aqui.
				SetConsoleTextAttribute(hOut, HLRED);
				printf("Erro no temporizador da Thread Detectora.\n");
			}
		}
	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLRED);
	printf("Thread Detectora encerrando execucao.\n");
	_endthreadex(0);
	return(0);
}//ThreadDetectora


//função que gera sequencias alfanumericas aleatorias
void gerarAlfaNumAleatorio(char* alfa, int len) {
	char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < len; i++) {
		alfa[i] = charSet[rand() % (sizeof(charSet) - 1)];
	}
	alfa[len] = 0;
}
