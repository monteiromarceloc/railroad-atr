#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_WARNINGS

#define TAM_ALARM 30+5
#define TAM_MSG 41+6

#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <queue>

using namespace std;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
typedef unsigned* CAST_LPDWORD;						// _beginthreadex

DWORD WINAPI DetectoraRodaQuente();		
DWORD WINAPI DepositaMensagens2();		
DWORD WINAPI DepositaMensagens1();		
DWORD WINAPI RetiradaMensagens();		

void gerarAlfaNumAleatorio(char* alfa, int len);

HANDLE hTimerQueue;
HANDLE hTimer1, hTimer2, hTimer3;
HANDLE hTimerEvent;
HANDLE hMutex;										// Permite acesso exclusicvo ao posicoes_livres
HANDLE hMutexNSEQ;									// Permite acesso exclusicvo ao NSEQ1

typedef struct {									
	char* conteudo;									
}Dados;


Dados Mensagens[200];								// Lista circular em memória

int NSEQ1 = 0;										// NSEQ referente aos Dados da Ferrovia
int NSEQ2 = 0;										// NSEQ referente a Deteccao de Roda Quente


HANDLE h1Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, "Leitura1ON-OFF");
HANDLE h2Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, "Leitura2ON-OFF");
HANDLE hEventDetector = OpenEvent(EVENT_ALL_ACCESS, FALSE, "DetectoraRodaQuenteON-OFF");
HANDLE hEventRetirada = OpenEvent(EVENT_ALL_ACCESS, FALSE, "RetiradaMensagensON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

HANDLE PosNova = CreateEvent(NULL, FALSE, FALSE, "PosNova");			// Evento para sinalizar uma posicao liberada na lista cheia

queue<int> PosLivres;								
queue<int> PosOcupada;					
SYSTEMTIME tempo1, tempo2;

int main()		
{
	BOOL status;

	DWORD dwIdDadosProcesso;
	DWORD dwIdOP;
	DWORD ret;

	HANDLE hThreads[4];						// Handles para as Threads secundárias

	int SetTemporizador, i, aux = 0;
	int nTipoEvento;

	for (i = 0; i < 100; i++) {
		PosLivres.push(aux);
		aux++;
	}

	hMutex = CreateMutex(NULL, FALSE, "Posicoes_Livres");
	hMutexNSEQ = CreateMutex(NULL, FALSE, "Numeros_Sequenciais");

	hThreads[0] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)DetectoraRodaQuente,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdDadosProcesso
	);

	hThreads[1] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)RetiradaMensagens,
		(LPVOID)1,
		0,
		(CAST_LPDWORD)&dwIdOP
	);

	hThreads[2] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)DepositaMensagens1,
		(LPVOID)2,
		0,
		(CAST_LPDWORD)&dwIdOP
	);

	hThreads[3] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)DepositaMensagens2,
		(LPVOID)3,
		0,
		(CAST_LPDWORD)&dwIdOP
	);

	HANDLE Threads[4] = { hThreads[0], hThreads[1], hThreads[2], hThreads[3] };

	ret = WaitForSingleObject(hEscEvent, INFINITE);

	WaitForMultipleObjects(4, hThreads, TRUE, INFINITE);

	CloseHandle(hMutex);
	CloseHandle(hMutexNSEQ);
	CloseHandle(hEventDetector);
	CloseHandle(hEventRetirada);
	CloseHandle(hEscEvent);

	for (i = 0; i < 4; i++) {
		CloseHandle(hThreads[i]);
	}

	return EXIT_SUCCESS;
}

DWORD WINAPI DetectoraRodaQuente() {

	int ID2, ESTADO, auxiliar, nTipoEvento, posicao_livre = 0;
	char buffer[TAM_ALARM], ID1[3];
	DWORD ret;

	HANDLE hEventDados = CreateEvent(NULL, TRUE, FALSE, "CriarMailslotDados");
	HANDLE hEventMailslotDados = CreateEvent(NULL, TRUE, FALSE, "MailslotDadosEVENTO");
	HANDLE hMailslot;
	HANDLE Events[2] = { hEventDetector, hEscEvent };
	DWORD dwBytesEnviados;

	SetEvent(hEventDados);
	WaitForSingleObject(hEventMailslotDados, INFINITE);

	hMailslot = CreateFile(
		"\\\\.\\mailslot\\MailslotDados",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 2000);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hEventDetector);
			cout << "Tarefa de retirada de dados: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hEventDetector); cout << "Tarefa de retirada de dados: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetLocalTime(&tempo2);
		NSEQ2++;
		if (NSEQ2 == 1000000) NSEQ2 = 1;
		gerarAlfaNumAleatorio(ID1, 3);
		ID2 = (rand() % 10000);
		ESTADO = (rand() % 2);
		sprintf(buffer, "%07d;00;%s-%04d;%d;%02d:%02d:%02d", NSEQ2, ID1, ID2, ESTADO, tempo2.wHour, tempo2.wMinute, tempo2.wSecond);

		WriteFile(hMailslot, &buffer, TAM_ALARM, &dwBytesEnviados, NULL);
		auxiliar = sizeof(buffer);

	} while (nTipoEvento != 1);

	CloseHandle(Events);
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslot);

	_endthreadex(0);
	return(0);
}

DWORD WINAPI RetiradaMensagens() {
	int auxiliar, nTipoEvento;
	char buffer[TAM_MSG];
	DWORD ret, dwBytesEnviados;
	LONG lOldValue;

	int posLivreArquivo = 0;

	HANDLE semaphore = CreateSemaphore(NULL, 200, 200, "SemaforoArquivoCheio");
	HANDLE hsemaphore = CreateSemaphore(NULL, 0, 200, "SemaforoArquivoLer");

	HANDLE hEventAlarme = CreateEvent(NULL, TRUE, FALSE, "CriarArquivoAlarme");
	HANDLE hAlarme = CreateEvent(NULL, TRUE, FALSE, "ArquivoCriado");
	HANDLE hArqMutex = CreateMutex(NULL, FALSE, "MutexArquivo");
	HANDLE Events[2] = { hEventRetirada, hEscEvent };
	HANDLE Events2[2] = { semaphore, hEscEvent };

	SetEvent(hEventAlarme);
	WaitForSingleObject(hAlarme, INFINITE);

	HANDLE hFile = CreateFile("..\\Files\\Mensagens.arq",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// abre para leitura e escrita
		NULL,								// atributos de seguran�a 
		OPEN_EXISTING,						// abre arquivo se j� criado 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 200);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hEventRetirada);
			cout << "Tarefa de retirada de alarmes: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hEventRetirada); cout << "Tarefa de retirada de alarmes: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}
		if (!PosOcupada.empty()) {
			auxiliar = PosOcupada.front();
			PosOcupada.pop();
			strcpy(buffer, Mensagens[auxiliar].conteudo);

			delete[]  Mensagens[auxiliar].conteudo;

			if (PosLivres.empty()) {
				PosLivres.push(auxiliar);
				PulseEvent(PosNova);
			}
			else if (!PosLivres.empty())
				PosLivres.push(auxiliar);
			
			if (buffer[14] == '9' && buffer[15] == '9' && buffer[16] == '9') {
				// ENVIAR MAILSLOT PARA THREAD DE EXIBIÇÃO DE ALARMES
			}

			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {}
			else if (nTipoEvento == 1) { break; }
			WaitForSingleObject(hArqMutex, INFINITE);
			SetFilePointer(hFile, (posLivreArquivo * TAM_MSG), NULL, FILE_BEGIN);
			WriteFile(hFile, buffer, sizeof(buffer), &dwBytesEnviados, NULL);
			FlushFileBuffers(hFile);
			ReleaseMutex(hArqMutex);
			ReleaseSemaphore(hsemaphore, 1, &lOldValue);

			if (lOldValue == 499) {
				cout << "Arquivo circular em disco esta cheio. Retirada de alarmes bloqueada. Aguardando nova posicao." << endl;
				ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
				nTipoEvento = ret - WAIT_OBJECT_0;
				if (nTipoEvento == 0) {}
				else if (nTipoEvento == 1) { break; }
			}

			posLivreArquivo++;
			if (posLivreArquivo == 500) {
				posLivreArquivo = 0;
			}

		}
	} while (nTipoEvento != 1);

	CloseHandle(Events);
	CloseHandle(Events2);
	CloseHandle(hsemaphore);
	CloseHandle(semaphore);
	CloseHandle(Events);
	CloseHandle(hEventAlarme);
	CloseHandle(hAlarme);
	CloseHandle(hArqMutex);

	_endthreadex(1);
	return(0);
}

DWORD WINAPI DepositaMensagens1()
{
	
	char buffer[TAM_MSG];
	int nTipoEvento;
	int ID2, ESTADO, REMOTA, DIAG, posicao_livre = 0, auxiliar ;
	char ID1[3];
	string ID;
	HANDLE Events[2] = { h1Event, hEscEvent };
	DWORD ret;

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(h1Event);
			cout << "Tarefa de leitura 1: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(h1Event); cout << "Tarefa de leitura 1: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		WaitForSingleObject(hMutexNSEQ, INFINITE);

		GetLocalTime(&tempo1);
		NSEQ1++;
		if (NSEQ1 == 1000000) NSEQ1 = 1;
		REMOTA = (rand() % 2);
		DIAG = (rand() % 1000);
		gerarAlfaNumAleatorio(ID1, 3);
		ID2 = (rand() % 1000);
		ID = "ABC-1234";
		ESTADO = (rand() % 2);
		sprintf(buffer, "%07d;55;%02d;%03d;%s-%04d;%d;%02d:%02d:%02d", NSEQ1, REMOTA, DIAG, ID1,ID2, ESTADO, tempo1.wHour, tempo1.wMinute, tempo1.wSecond);
		ReleaseMutex(hMutexNSEQ);

		auxiliar = sizeof(buffer);

		WaitForSingleObject(hMutex, INFINITE);
		if (PosLivres.empty()) {
			cout << "A lista circular em memoria esta cheia. Thread Leitura 1 bloqueada. Aguardando posicao livre." << endl;
			WaitForSingleObject(PosNova, INFINITE);
		}

		posicao_livre = PosLivres.front();
		PosLivres.pop();

		Mensagens[posicao_livre].conteudo = new char[auxiliar];
		strcpy(Mensagens[posicao_livre].conteudo, buffer);
		PosOcupada.push(posicao_livre);
		ReleaseMutex(hMutex);

	} while (nTipoEvento != 1);

	CloseHandle(Events);

	_endthreadex(0);
	return(0);
};

DWORD WINAPI DepositaMensagens2()
{

	char buffer[TAM_MSG];
	int nTipoEvento;
	int ID2, ESTADO, REMOTA, DIAG, posicao_livre = 0, auxiliar;
	char ID1[3];
	HANDLE Events[2] = { h2Event, hEscEvent };
	DWORD ret;

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(h2Event);
			cout << "Tarefa de leitura 2: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(h2Event); cout << "Tarefa de leitura 2: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		WaitForSingleObject(hMutexNSEQ, INFINITE);

		GetLocalTime(&tempo1);
		NSEQ1++;
		if (NSEQ1 == 1000000) NSEQ1 = 1;
		REMOTA = (rand() % 2);
		DIAG = (rand() % 1000);
		gerarAlfaNumAleatorio(ID1, 3);
		ID2 = (rand() % 1000);
		ESTADO = (rand() % 2);
		sprintf(buffer, "%07d;55;%02d;%03d;%s-%04d;%d;%02d:%02d:%02d", NSEQ1, REMOTA, DIAG, ID1, ID2, ESTADO, tempo1.wHour, tempo1.wMinute, tempo1.wSecond);
		ReleaseMutex(hMutexNSEQ);

		auxiliar = sizeof(buffer);

		WaitForSingleObject(hMutex, INFINITE);
		if (PosLivres.empty()) {
			cout << "A lista circular em memoria esta cheia. Tarefa Leitura 2 bloqueada. Aguardando posicao livre." << endl;
			WaitForSingleObject(PosNova, INFINITE);
		}

		posicao_livre = PosLivres.front();
		PosLivres.pop();

		Mensagens[posicao_livre].conteudo = new char[auxiliar];
		strcpy(Mensagens[posicao_livre].conteudo, buffer);
		PosOcupada.push(posicao_livre);
		ReleaseMutex(hMutex);

	} while (nTipoEvento != 1);

	CloseHandle(Events);

	_endthreadex(0);
	return(0);
};

//função que gera sequencias alfanumericas aleatorias
void gerarAlfaNumAleatorio(char* alfa, int len) {
	char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < len; i++) {
		alfa[i] = charSet[rand() % (sizeof(charSet) - 1)];
	}
	alfa[len] = 0;
}