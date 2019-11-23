#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <process.h>	// _beginthreadex() e _endthreadex() 
#include <conio.h>		// _getch
#include <iostream>

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE

using namespace std;
	
#define h		0x68
#define r		0x72
#define s		0x73
#define a		0x61
#define	ESC		0x1B

HANDLE h1Event;			// Handle para Evento Leitura Remota 1 on/off
HANDLE h2Event;			// Handle para Evento Leitura Remota 2 on/off
HANDLE hhEvent;			// Handle para Evento Leitura dos Detectores de Roda Quente on/off
HANDLE hrEvent;			// Handle para Evento Retirada de Mensagens on/off
HANDLE hsEvent;			// Handle para Evento Exibicao de Dados de Sinalização Ferroviária on/off
HANDLE haEvent;			// Handle para Evento Exibicao de Alarmes on/offNewProcess
HANDLE hEscEvent;		// Handle para Evento Encerrar demais tarefas
HANDLE hOut;							// Handle para a saída da console

int main() {

	int nTecla;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) {
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Erro ao obter handle para a saída da console\n");
	}

	STARTUPINFO si;					// StartUpInformation para novo processo 
	PROCESS_INFORMATION RailroadProcess, DetectoresProcess, ExibicaoDadosProcess, ExibicaoAlarmeProcess;	// Informações sobre os novos processos criados 

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);				// Tamanho da estrutura em bytes


	h1Event = CreateEvent(NULL, TRUE, FALSE, "Leitura1ON-OFF");
	h2Event = CreateEvent(NULL, TRUE, FALSE, "Leitura2ON-OFF");
	hhEvent = CreateEvent(NULL, TRUE, FALSE, "LeituraDetectoresON-OFF");
	hrEvent = CreateEvent(NULL, TRUE, FALSE, "RetiradaMensagensON-OFF");
	hsEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoDadosON-OFF");
	haEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoAlarmesON-OFF");
	hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EncerraTarefas");

	CreateProcess(
		"..\\Debug\\ControleLista.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processow
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		//CREATE_NEW_CONSOLE,	
		NORMAL_PRIORITY_CLASS,	// CreationFlags
		NULL,	// lpEnvironment
		"..\\ControleLista",
		&si,			// lpStartUpInfo
		&RailroadProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Detectores.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Detectores de roda quente",
		&si,			// lpStartUpInfo
		&DetectoresProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\ExibicaoDados.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de dados",
		&si,			// lpStartUpInfo
		&ExibicaoDadosProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\ExibicaoAlarme.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de alarmes",
		&si,			// lpStartUpInfo
		&ExibicaoAlarmeProcess);	// lpProcessInformation


	HANDLE EndProcess[4] = { RailroadProcess.hProcess, DetectoresProcess.hProcess, ExibicaoDadosProcess.hProcess, ExibicaoAlarmeProcess.hProcess };

	SetConsoleTextAttribute(hOut, WHITE);
	printf("Tecle uma acao valida para gerar evento ou <Esc> para terminar\n");

	do {
		
		//espera uma tecla ser digitada
		nTecla = _getch();
		if (nTecla == '1') SetEvent(h1Event);	// Leitura1ON-OFF
		if (nTecla == '2') SetEvent(h2Event); 	// Leitura1ON-OFF
		if (nTecla == h)  SetEvent(hhEvent); 	// LeituraDetectoresON-OFF
		if (nTecla == r) SetEvent(hrEvent);		// RetiradaMensagensON-OFF
		if (nTecla == s) SetEvent(hsEvent);		// ExibicaoDadosON-OFF
		if (nTecla == a) SetEvent(haEvent);		// ExibicaoAlarmesON-OFF
		else if (nTecla == ESC) SetEvent(hEscEvent);	// EncerraTarefas
	} while (nTecla != ESC);

	WaitForMultipleObjects(4, EndProcess, TRUE, INFINITE);

	CloseHandle(h1Event);
	CloseHandle(h2Event);
	CloseHandle(hhEvent);
	CloseHandle(hrEvent);
	CloseHandle(hsEvent);
	CloseHandle(haEvent);
	CloseHandle(hEscEvent);

	CloseHandle(RailroadProcess.hProcess);
	CloseHandle(RailroadProcess.hThread);

	CloseHandle(DetectoresProcess.hProcess);
	CloseHandle(DetectoresProcess.hThread);

	CloseHandle(ExibicaoDadosProcess.hProcess);
	CloseHandle(ExibicaoDadosProcess.hThread);

	CloseHandle(ExibicaoAlarmeProcess.hProcess);
	CloseHandle(ExibicaoAlarmeProcess.hThread);

	SetConsoleTextAttribute(hOut, WHITE);
	printf("\nAcione uma tecla para terminar\n");
	_getch();

	return EXIT_SUCCESS;

}