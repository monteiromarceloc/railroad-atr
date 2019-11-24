#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_WARNINGS
#define TAM_MSG 30
#define TAM_ALARM 41

#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <queue>				

using namespace std;

typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
typedef unsigned *CAST_LPDWORD;						// _beginthreadex

	
DWORD WINAPI RetiradaDados();					    // Thread representando a TAREFA DE RETIRADA DE DADOS DO PROCESSO
DWORD WINAPI RetiradaOP();							// Thread representando a TAREFA DE RETIRADA DE ORDENS DE PRODUÇÃO
DWORD WINAPI RetiradaAlarmes();						// Thread representando a TAREFA DE RETIRADA DE ALARMES

void CALLBACK DepositaDados(PVOID, BOOLEAN);	    // Rotina callback para o depósito dos dados do processo
void CALLBACK DepositaOP(PVOID, BOOLEAN);			// Rotina callback para o depósito das OP's
void CALLBACK DepositaAlarmes(PVOID, BOOLEAN);		// Rotina callback para o depósito dos alarmes

HANDLE hTimerQueue;
HANDLE hTimer1, hTimer2, hTimer3;
HANDLE hTimerEvent;
HANDLE hMutex;										// Permite acesso exclusicvo a queue contendo os valores de posições livres

typedef struct {									// Estrutura de dados que armazenara os conteúdos colocado na lista circular em memoria,
	char *conteudo;									// será alocado dinamicamente 
}Dados;

queue<int> PosLivres;								// Lista que conterá todas as posições livres na lista circular em memoria 
queue<int> PosOcupadaDados;							// Posicao em que devera ser feito a retirada das mensagens Dados de Processo
queue<int> PosOcupadaOP;							// Posicao em que devera ser feito a retirada das mensagns OPs
queue<int> PosOcupadaAlarmes;						// Posicao em que devera ser feito a retirada das mensagens Alarmes

Dados Mensagens[100];								// Lista circular em memória

int NSEQ1 = 0;										// NSEQ referente aos Dados de Processo
int NSEQ2 = 0;										// NSEQ referente aos Alarmes	
int NSEQ3 = 0;										// NSEQ referente as OPs

SYSTEMTIME tempo1, tempo2, tempo3;

HANDLE hdEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ComunicacaoDadosON-OFF");
HANDLE hpEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "RetiradaDadosON-OFF");
HANDLE hoEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "RetiradaOPsON-OFF");
HANDLE haEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "RetiradaAlarmesON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

HANDLE PosNova = CreateEvent(NULL, FALSE, FALSE, "PosNova");			// Evento para sinalizar uma posicao, para quando
																		// a lista estiver cheia e liberar posicoes


int main()		// Thread primária - representa a TAREFA DE COMUNICAÇÃO DE DADOS
{
	BOOL status;

	DWORD dwIdDadosProcesso;
	DWORD dwIdOP;
	DWORD ret;

	HANDLE hThreads[3];							// Handles para as Threads secundárias
	HANDLE Events[2] = {hdEvent, hEscEvent};


	int SetTemporizador, i, aux = 0;
	int nTipoEvento;

	for (i = 0; i < 100; i++) {
		PosLivres.push(aux);
		aux++;
	}

	//go away
	SetTemporizador = 1000 + (rand() % 4000);		// Tempo para o depósito da primeira OP, valor entre 1000 e 5000 ms

	hTimerQueue = CreateTimerQueue();
	if (hTimerQueue == NULL) {
		cout << "Falha em CreateTimerQueue! Codigo =%d)\n" << GetLastError();
		return 0;
	}

	hMutex = CreateMutex(NULL, FALSE, "Posicoes_Livres");

	hThreads[0] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)RetiradaDados,	// casting necessário
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdDadosProcesso	// cating necessário
	);

	hThreads[1] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)RetiradaOP,	// casting necessário
		(LPVOID)1,
		0,
		(CAST_LPDWORD)&dwIdOP	// cating necessário
	);

	hThreads[2] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)RetiradaAlarmes,	// casting necessário
		(LPVOID)2,
		0,
		(CAST_LPDWORD)&dwIdOP	// cating necessário
	);

	HANDLE Threads[3] = { hThreads[0], hThreads[1], hThreads[2] };

	status = CreateTimerQueueTimer(&hTimer1, hTimerQueue, (WAITORTIMERCALLBACK)DepositaDados,
		NULL, 500, 500, WT_EXECUTEDEFAULT);
	if(!status){
		cout << "Erro em CreateTimerQueueTimer [1]! Codigo = " << GetLastError();
		return 0;
	}

	status = CreateTimerQueueTimer(&hTimer2, hTimerQueue, (WAITORTIMERCALLBACK)DepositaAlarmes,
		NULL, 500, 500, WT_EXECUTEDEFAULT);
	if (!status) {
		cout << "Erro em CreateTimerQueueTimer [2]! Codigo = " << GetLastError() << endl;
		return 0;
	}

	status = CreateTimerQueueTimer(&hTimer3, hTimerQueue, (WAITORTIMERCALLBACK)DepositaOP,
		NULL, 1000, SetTemporizador, WT_EXECUTEDEFAULT);
	if (!status) {
		cout << "Erro em CreateTimerQueueTimer [3]! Codigo = " << GetLastError() << endl;
		return 0;
	}

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hdEvent);
			cout << "Tarefa de comunicacao de dados: OFF" << endl;
			DeleteTimerQueueTimer(hTimerQueue, hTimer1, NULL);
			DeleteTimerQueueTimer(hTimerQueue, hTimer2, NULL);
			DeleteTimerQueueTimer(hTimerQueue, hTimer3, NULL);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				ResetEvent(hdEvent);
				cout << "Tarefa de comunicacao de dados: ON" << endl;
				CreateTimerQueueTimer(&hTimer1, hTimerQueue, (WAITORTIMERCALLBACK)DepositaDados,
					NULL, 1000, 500, WT_EXECUTEDEFAULT);
				CreateTimerQueueTimer(&hTimer2, hTimerQueue, (WAITORTIMERCALLBACK)DepositaAlarmes,
					NULL, 1000, 1000, WT_EXECUTEDEFAULT);
				CreateTimerQueueTimer(&hTimer3, hTimerQueue, (WAITORTIMERCALLBACK)DepositaOP,
					NULL, 1000, SetTemporizador, WT_EXECUTEDEFAULT);
			}
			else if (nTipoEvento == 1) { 
				WaitForMultipleObjects(3, Threads, TRUE, INFINITE);
				break;
			}
		}
		else if (nTipoEvento == 1) { break; }
	} while (nTipoEvento != 1);

	WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);

	CloseHandle(hMutex);
	CloseHandle(hdEvent);
	CloseHandle(hpEvent);
	CloseHandle(hoEvent);
	CloseHandle(haEvent);
	CloseHandle(hEscEvent);
	CloseHandle(Events);

	for (i = 0; i < 3; i++) {
		CloseHandle(hThreads[i]);
	}
	
	if (!DeleteTimerQueueEx(hTimerQueue, NULL))
		cout << "Falha em DeleteTimerQueue! Codigo = " << GetLastError() << endl;

	return EXIT_SUCCESS;
}

DWORD WINAPI RetiradaDados() {	
	
	int ID, ESTADO, auxiliar, nTipoEvento, posicao_livre=0;
	char buffer[TAM_MSG];
	DWORD ret; 

	HANDLE hEventDados = CreateEvent(NULL, TRUE, FALSE, "CriarMailslotDados");
	HANDLE hEventMailslotDados = CreateEvent(NULL, TRUE, FALSE, "MailslotDadosEVENTO");
	HANDLE hMailslot;
	HANDLE Events[2] = {hpEvent, hEscEvent};
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
			ResetEvent(hpEvent);
			cout << "Tarefa de retirada de dados: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0){ ResetEvent(hpEvent); cout << "Tarefa de retirada de dados: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) { 
			break; }

		GetLocalTime(&tempo2);
		NSEQ2++;
		if (NSEQ2 == 1000000) NSEQ2 = 1;
		ID = (rand() % 10000);
		ESTADO = (rand() % 4 == 3 ? 1 : 0);
		sprintf(buffer, "%06d;00;%04d;%d;%02d:%02d:%02d", NSEQ2, ID, ESTADO, tempo2.wHour, tempo2.wMinute, tempo2.wSecond);

		auxiliar = sizeof(buffer);

		WaitForSingleObject(hMutex, INFINITE);
		if (PosLivres.empty()) {
			;
			cout << "A lista circular em memoria esta cheia. Tarefa Comunicacao de Dados bloqueada. Aguardando posicao livre." << endl;
			WaitForSingleObject(PosNova, INFINITE);
		}

		posicao_livre = PosLivres.front();
		PosLivres.pop();
		ReleaseMutex(hMutex);

		Mensagens[posicao_livre].conteudo = new char[auxiliar];
		strcpy(Mensagens[posicao_livre].conteudo, buffer);
		PosOcupadaDados.push(posicao_livre);



		if (!PosOcupadaDados.empty()) {
			auxiliar = PosOcupadaDados.front();
			PosOcupadaDados.pop();
			strcpy(buffer, Mensagens[auxiliar].conteudo);

			delete[]  Mensagens[auxiliar].conteudo;

			if (PosLivres.empty()) {
				PosLivres.push(auxiliar);
				PulseEvent(PosNova);
			}else if(!PosLivres.empty())
				PosLivres.push(auxiliar);


			WriteFile(hMailslot, &buffer, TAM_MSG, &dwBytesEnviados, NULL);
			//ENVIAR MENSAGEM PARA OUTRA TAREFA
		}
	} while (nTipoEvento != 1);

	CloseHandle(Events);
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslot);
	
	_endthreadex(0);
	return(0);
}

DWORD WINAPI RetiradaAlarmes() {
	int auxiliar, nTipoEvento;
	char buffer[TAM_ALARM];
	DWORD ret, dwBytesEnviados;
	LONG lOldValue;

	int posLivreArquivo = 0;

	HANDLE semaphore = CreateSemaphore(NULL, 200, 200, "SemaforoArquivoCheio");
	HANDLE hsemaphore = CreateSemaphore(NULL, 0, 200, "SemaforoArquivoLer");

	HANDLE hEventAlarme = CreateEvent(NULL, TRUE, FALSE, "CriarArquivoAlarme");
	HANDLE hAlarme = CreateEvent(NULL, TRUE, FALSE, "ArquivoCriado");
	HANDLE hArqMutex = CreateMutex(NULL, FALSE, "MutexArquivo");
	HANDLE Events[2] = { haEvent, hEscEvent };
	HANDLE Events2[2] = { semaphore, hEscEvent };

	SetEvent(hEventAlarme);
	WaitForSingleObject(hAlarme, INFINITE);

	HANDLE hFile = CreateFile("..\\Alarme\\Alarmes.arq",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// abre para leitura e escrita
		NULL,								// atributos de seguran�a 
		OPEN_EXISTING,						// abre arquivo se j� criado 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 1000);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(haEvent);
			cout << "Tarefa de retirada de alarmes: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(haEvent); cout << "Tarefa de retirada de alarmes: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}
		if (!PosOcupadaAlarmes.empty()) {
			auxiliar = PosOcupadaAlarmes.front();
			PosOcupadaAlarmes.pop();
			strcpy(buffer, Mensagens[auxiliar].conteudo);

			delete[]  Mensagens[auxiliar].conteudo;

			if (PosLivres.empty()) {
				PosLivres.push(auxiliar);
				PulseEvent(PosNova);
			}
			else if (!PosLivres.empty())
				PosLivres.push(auxiliar);
			//ENVIAR MENSAGEM PARA OUTRA TAREFA

			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {}
			else if (nTipoEvento == 1) { break; }
			WaitForSingleObject(hArqMutex, INFINITE);
			SetFilePointer(hFile, (posLivreArquivo * TAM_ALARM), NULL, FILE_BEGIN);
			WriteFile(hFile, buffer, sizeof(buffer), &dwBytesEnviados, NULL);
			FlushFileBuffers(hFile);
			ReleaseMutex(hArqMutex);
			ReleaseSemaphore(hsemaphore, 1, &lOldValue);

			if (lOldValue == 199) {
				cout << "Arquivo circular em disco esta cheio. Retirada de alarmes bloqueada. Aguardando nova posicao." << endl;
				ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
				nTipoEvento = ret - WAIT_OBJECT_0;
				if (nTipoEvento == 0) {}
				else if (nTipoEvento == 1) { break; }
			}

			posLivreArquivo++;
			if (posLivreArquivo == 200) {
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

DWORD WINAPI RetiradaOP() {
	int auxiliar, nTipoEvento;
	char buffer[35];
	DWORD ret;

	HANDLE hEventOP = CreateEvent(NULL, TRUE, FALSE, "CriarMailslotOP");
	HANDLE hEventMailslotOP = CreateEvent(NULL, TRUE, FALSE, "MailslotOPEVENTO");
	HANDLE hMailslotOP;
	HANDLE Events[2] = { hoEvent, hEscEvent };
	DWORD dwBytesEnviados;

	SetEvent(hEventOP);
	WaitForSingleObject(hEventMailslotOP, INFINITE);

	hMailslotOP = CreateFile(
		"\\\\.\\mailslot\\MailslotOP",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 1000);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hoEvent);
			cout << "Tarefa de retirada de OP: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hoEvent); cout << "Tarefa de retirada de OP: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			 break;
		}
		if (!PosOcupadaOP.empty()) {
			auxiliar = PosOcupadaOP.front();
			PosOcupadaOP.pop();
			strcpy(buffer, Mensagens[auxiliar].conteudo);

			delete[]  Mensagens[auxiliar].conteudo;
			
			if (PosLivres.empty()) {
				PosLivres.push(auxiliar);
				PulseEvent(PosNova);
			}else if(!PosLivres.empty())
				PosLivres.push(auxiliar);

			//ENVIAR MENSAGEM PARA OUTRA TAREFA
			WriteFile(hMailslotOP, &buffer, 35, &dwBytesEnviados, NULL);
		}
	} while (nTipoEvento != 1);

	CloseHandle(Events);
	CloseHandle(hEventOP);
	CloseHandle(hEventMailslotOP);
	CloseHandle(hMailslotOP);

	_endthreadex(2);
	return(0);
}

void CALLBACK DepositaDados(PVOID nTimerID, BOOLEAN TimerOrWaitFired)
{/*
	int ESTADO,ID, posicao_livre = 0;
	char buffer[TAM_MSG];

	int auxiliar, nTipoEvento;
	DWORD ret;

	HANDLE hEventDados = CreateEvent(NULL, TRUE, FALSE, "CriarMailslotDados");
	HANDLE hEventMailslotDados = CreateEvent(NULL, TRUE, FALSE, "MailslotDadosEVENTO");
	HANDLE hMailslot;
	HANDLE Events[2] = { hpEvent, hEscEvent };
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
			ResetEvent(hpEvent);
			cout << "Tarefa de retirada de dados: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hpEvent); cout << "Tarefa de retirada de dados: ON" << endl; }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetLocalTime(&tempo2);
		NSEQ2++;
		if (NSEQ2 == 1000000) NSEQ2 = 1;
		ID = (rand() % 10000);
		ESTADO = (rand() % 4 == 3 ? 1 : 0);
		sprintf(buffer, "%06d;00;%04d;%d;%02d:%02d:%02d", NSEQ2, ID, ESTADO, tempo2.wHour, tempo2.wMinute, tempo2.wSecond);

		auxiliar = sizeof(buffer);

		WaitForSingleObject(hMutex, INFINITE);
		if (PosLivres.empty()) {
			;
			cout << "A lista circular em memoria esta cheia. Tarefa Comunicacao de Dados bloqueada. Aguardando posicao livre." << endl;
			WaitForSingleObject(PosNova, INFINITE);
		}

		posicao_livre = PosLivres.front();
		PosLivres.pop();
		ReleaseMutex(hMutex);

		Mensagens[posicao_livre].conteudo = new char[auxiliar];
		strcpy(Mensagens[posicao_livre].conteudo, buffer);
		PosOcupadaDados.push(posicao_livre);

		if (!PosOcupadaDados.empty()) {
			auxiliar = PosOcupadaDados.front();
			PosOcupadaDados.pop();
			strcpy(buffer, Mensagens[auxiliar].conteudo);

			delete[]  Mensagens[auxiliar].conteudo;

			if (PosLivres.empty()) {
				PosLivres.push(auxiliar);
				PulseEvent(PosNova);
			}
			else if (!PosLivres.empty())
				PosLivres.push(auxiliar);


			WriteFile(hMailslot, &buffer, TAM_MSG, &dwBytesEnviados, NULL);
			//ENVIAR MENSAGEM PARA OUTRA TAREFA
		}
	} while (nTipoEvento != 1);

	CloseHandle(Events);
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslot);

	*/
};

void CALLBACK DepositaAlarmes(PVOID nTimerID, BOOLEAN TimerOrWaitFired)
{
	char buffer[TAM_ALARM];
	int ID, ESTADO, REMOTA, DIAG, posicao_livre = 0, auxiliar;
	char* ID1;

	GetLocalTime(&tempo1);
	NSEQ1++;
	if (NSEQ1 == 1000000) NSEQ1 = 1;
	REMOTA = (rand() % 2);
	DIAG = (rand() % 1000);
	//gerarAlfaNumAleatorio(ID_PARTE1, 3);
	//ID_PARTE2 = (rand() % 1000);
	ESTADO = (rand() % 2);

	sprintf(buffer, "%07d;55;%02d;%03d;ID;%d;%02d:%02d:%02d", NSEQ1, REMOTA, DIAG, ESTADO, tempo1.wHour, tempo1.wMinute, tempo1.wSecond);


	auxiliar = sizeof(buffer);

	WaitForSingleObject(hMutex, INFINITE);
	if (PosLivres.empty()) {
		cout << "A lista circular em memoria esta cheia. Tarefa Comunicacao de Dados bloqueada. Aguardando posicao livre." << endl;
		WaitForSingleObject(PosNova, INFINITE);
	}

	posicao_livre = PosLivres.front();
	PosLivres.pop();
	ReleaseMutex(hMutex);

	Mensagens[posicao_livre].conteudo = new char[auxiliar];
	strcpy(Mensagens[posicao_livre].conteudo, buffer);
	PosOcupadaAlarmes.push(posicao_livre);
};

void CALLBACK DepositaOP(PVOID nTimerID, BOOLEAN TimerOrWaitFired)
{
	char buffer[35];
	int NOP, SETOR, RECEITA, posicao_livre = 0, SetTemporizador, auxiliar;

	GetLocalTime(&tempo3);

	NSEQ3++;
	if (NSEQ3 == 1000000) NSEQ3 = 1;

	NOP = (rand() % 99999999);
	SETOR = (rand() % 99);
	RECEITA = (rand() % 999);

	sprintf(buffer, "00|%06d|%08d|%02d|%03d|%02d:%02d:%02d", NSEQ3, NOP, SETOR, RECEITA, tempo3.wHour,
		tempo3.wMinute, tempo3.wSecond);

	auxiliar = sizeof(buffer);

	WaitForSingleObject(hMutex, INFINITE);

	if (PosLivres.empty()) {
		cout << "A lista circular em memoria esta cheia. Tarefa Comunicacao de Dados bloqueada. Aguardando posicao livre." << endl;
		WaitForSingleObject(PosNova, INFINITE);
	}
		

	posicao_livre = PosLivres.front();
	PosLivres.pop();

	ReleaseMutex(hMutex);

	Mensagens[posicao_livre].conteudo = new char[auxiliar];
	strcpy(Mensagens[posicao_livre].conteudo, buffer);
	PosOcupadaOP.push(posicao_livre);

	SetTemporizador = 1000 + (rand() % 4000);
	ChangeTimerQueueTimer(hTimerQueue, hTimer3, SetTemporizador, SetTemporizador);		// Atualização do temporizador de depósito 
																						// de mensagens, valores entre 1 e 5s	
};