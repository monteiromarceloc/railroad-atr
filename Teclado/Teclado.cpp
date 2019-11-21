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

#define h			0x68
#define r			0x72
#define s			0x73
#define a			0x61
#define	ESC			0x1B

HANDLE h1Event;			// Handle para Evento Leitura Remota 1 on/off
HANDLE h2Event;			// Handle para Evento Leitura Remota 2 on/off
HANDLE hhEvent;			// Handle para Evento Leitura dos Detectores de Roda Quente on/off
HANDLE hrEvent;			// Handle para Evento Retirada de Mensagens on/off
HANDLE hsEvent;			// Handle para Evento Exibicao de Dados de Sinaliza��o Ferrovi�ria on/off
HANDLE haEvent;			// Handle para Evento Exibicao de Alarmes on/off
HANDLE hEscEvent;		// Handle para Evento Encerrar demais tarefas

int main() {

	int nTecla;

	STARTUPINFO si;					// StartUpInformation para novo processo 
	PROCESS_INFORMATION NewProcess, DadosProcess, OPProcess, AlarmeProcess;	// Informa��es sobre os novos processos criados 

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
		"..\\Debug\\Controle De Dados.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		//CREATE_NEW_CONSOLE,	
		NORMAL_PRIORITY_CLASS,	// CreationFlags
		NULL,	// lpEnvironment
		"..\\Controle de Dados",
		&si,			// lpStartUpInfo
		&NewProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de Dados.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de Dados",
		&si,			// lpStartUpInfo
		&DadosProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de OPs.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de Ops",
		&si,			// lpStartUpInfo
		&OPProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de Alarmes.exe",
		NULL,	// linha de comando
		NULL,	// atributos de seguran�a: Processo
		NULL,	// atributos de seguran�a: Thread
		FALSE,	// heran�a de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de Alarmes",
		&si,			// lpStartUpInfo
		&AlarmeProcess);	// lpProcessInformation

	HANDLE EndProcess[4] = { NewProcess.hProcess, DadosProcess.hProcess, OPProcess.hProcess, AlarmeProcess.hProcess };

	do {
		printf("Tecle uma acao valida para gerar evento ou <Esc> para terminar\n");
		nTecla = _getch();
		if (nTecla == 1) SetEvent(h1Event);			// Gera 1 evento
		if (nTecla == 2) SetEvent(h2Event);			// Gera 1 evento
		if (nTecla == h) SetEvent(hhEvent);			// Gera 1 evento
		if (nTecla == r) SetEvent(hrEvent);			// Gera 1 evento
		if (nTecla == s) SetEvent(hsEvent);			// Gera 1 evento
		if (nTecla == a) SetEvent(haEvent);			// Gera 1 evento
		else if (nTecla == ESC) SetEvent(hEscEvent);	// Termina processos
	} while (nTecla != ESC);

	WaitForMultipleObjects(4, EndProcess, TRUE, INFINITE);

	CloseHandle(h1Event);
	CloseHandle(h2Event);
	CloseHandle(hhEvent);
	CloseHandle(hrEvent);
	CloseHandle(hsEvent);
	CloseHandle(haEvent);
	CloseHandle(hEscEvent);

	CloseHandle(NewProcess.hProcess);
	CloseHandle(NewProcess.hThread);

	CloseHandle(DadosProcess.hProcess);
	CloseHandle(DadosProcess.hThread);

	CloseHandle(OPProcess.hProcess);
	CloseHandle(OPProcess.hThread);

	CloseHandle(AlarmeProcess.hProcess);
	CloseHandle(AlarmeProcess.hThread);

	printf("\nAcione uma tecla para terminar\n");
	_getch();

	return EXIT_SUCCESS;

}