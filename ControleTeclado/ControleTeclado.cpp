#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <process.h>	// _beginthreadex() e _endthreadex() 
#include <conio.h>		// _getch

#include <iostream>
using namespace std;

#define n1			0x31
#define n2			0x32
#define a			0x61
#define s			0x73
#define r			0x72
#define h			0x68
#define	ESC			0x1B

HANDLE h1Event;			// Handle para Evento Comunicao de Dados on/off
HANDLE h2Event;			// Handle para Evento Comunicao de Dados on/off
HANDLE hpEvent;			// Handle para Evento Retirada de Dados on/off
HANDLE hoEvent;			// Handle para Evento Retirada de OPs on/off
HANDLE haEvent;			// Handle para Evento Retirada de Alarmes on/off
HANDLE hsEvent;			// Handle para Evento Exibicao de Dados on/off
HANDLE heEvent;			// Handle para Evento Exibicao de OPs on/off
HANDLE hwEvent;			// Handle para Evento Exibicao de Alarmes on/off
HANDLE hcEvent;			// Handle para Evento Limpeza tela de Alarmes
HANDLE hEscEvent;		// Handle para Evento Aborta


void cabecalho() {
	printf("\n     SALA DE CONTROLE - PRINCIPAL   \n\n");
	printf("------------------------------------------------\n\nCOMANDOS DISPONIVEIS:\n");
	printf("\n1- LEITURA 1 \t\t2- LEITURA 2 \nH- DETECTOR RODA QUENTE\tA- EXIB. ALARMES\nS- EXIB. DADOS\t\tR- RETIRADA MENSAGENS\nESC- FINALIZA PROCESSOS\n\n");
}//cabecalho*/


int main() {

	int nTecla;

	STARTUPINFO si;					// StartUpInformation para novo processo
	PROCESS_INFORMATION ProcessLeitura, ProcessAlarmes, ProcessMensagens;	// Informa��es sobre os novos processos criados

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);				// Tamanho da estrutura em bytes

	h1Event = CreateEvent(NULL, TRUE, FALSE, "Leitura1ON-OFF");
	h2Event = CreateEvent(NULL, TRUE, FALSE, "Leitura2ON-OFF");
	hpEvent = CreateEvent(NULL, TRUE, FALSE, "DetectoraRodaQuenteON-OFF");
	haEvent = CreateEvent(NULL, TRUE, FALSE, "RetiradaMensagensON-OFF");
	hsEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoAlarmesON-OFF");
	hwEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoMensagensON-OFF");
	hcEvent = CreateEvent(NULL, TRUE, FALSE, "ClearMensagens");
	hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EscEvento");

	if (!CreateProcess(
		"..\\Debug\\ControleLeitura.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		//CREATE_NEW_CONSOLE,	
		NORMAL_PRIORITY_CLASS,	// CreationFlags
		NULL,	// lpEnvironment
		"..\\ControleLeitura",
		&si,			// lpStartUpInfo
		&ProcessLeitura))	// lpProcessInformation
		printf("\nFailToCreate %d\n", GetLastError());


	if(!CreateProcess(
		"..\\Debug\\ExibeAlarmes.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\ExibeAlarmes",
		&si,			// lpStartUpInfo
		&ProcessAlarmes))	// lpProcessInformation
	printf("\nFailToCreate %d\n", GetLastError());


	CreateProcess(
		"..\\Debug\\ExibeMensagens.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\ExibeMensagens",
		&si,			// lpStartUpInfo
		&ProcessMensagens);	// lpProcessInformation

	HANDLE EndProcess[3] = { ProcessLeitura.hProcess, ProcessAlarmes.hProcess, ProcessMensagens.hProcess };

	cabecalho();

	do {
		nTecla = _getch();
		if (nTecla == n1) SetEvent(h1Event);			// LEITURA 1
		else if (nTecla == n2) SetEvent(h2Event);		// LEITURA 2
		else if (nTecla == h) SetEvent(hpEvent);		// RODA QUENTE
		else if (nTecla == r) SetEvent(haEvent);		// RETIRADA DE MENSAGENS
		else if (nTecla == s) SetEvent(hwEvent);		// EXIBIÇÃO DE DADOS (MENSAGENS)
		else if (nTecla == a) SetEvent(hsEvent);		// EXIBIÇÃO DE ALARMES
		else if (nTecla == ESC) SetEvent(hEscEvent);	// EscEvento
		else printf("Tecle uma acao valida para gerar evento ou <Esc> para terminar\n");
	} while (nTecla != ESC);

	WaitForMultipleObjects(4, EndProcess, TRUE, INFINITE);

	CloseHandle(hpEvent);
	CloseHandle(hoEvent);
	CloseHandle(haEvent);
	CloseHandle(hsEvent);
	CloseHandle(heEvent);
	CloseHandle(hwEvent);
	CloseHandle(hcEvent);
	CloseHandle(hEscEvent);

	CloseHandle(ProcessLeitura.hProcess);
	CloseHandle(ProcessLeitura.hThread);

	CloseHandle(ProcessAlarmes.hProcess);
	CloseHandle(ProcessAlarmes.hThread);

	CloseHandle(ProcessMensagens.hProcess);
	CloseHandle(ProcessMensagens.hThread);

	printf("\nAcione uma tecla para terminar\n");
	_getch();

	return EXIT_SUCCESS;

}