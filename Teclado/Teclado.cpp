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

#define num 1		
#define num 2		
#define h		0x68
#define r		0x72
#define s		0x73
#define a		0x61
#define	ESC		0x1B

HANDLE hEventLeitura1;			// Handle para Evento Leitura Remota 1 on/off
HANDLE hEventLeitura2;			// Handle para Evento Leitura Remota 2 on/off
HANDLE hEventDetect;			// Handle para Evento Leitura dos Detectores de Roda Quente on/off
HANDLE hEventRetirada;			// Handle para Evento Retirada de Mensagens on/off
HANDLE hsEvent;			// Handle para Evento Exibicao de Dados de Sinalização Ferroviária on/off
HANDLE haEvent;			// Handle para Evento Exibicao de Alarmes on/offNewProcess
HANDLE hEscEvent;		// Handle para Evento Encerrar demais tarefas

int main() {

	int nTecla;

	STARTUPINFO si;					// StartUpInformation para novo processo 
	PROCESS_INFORMATION RailroadProcess, DetectoresProcess, ExibicaoDadosProcess, ExibicaoAlarmeProcess;	// Informações sobre os novos processos criados 

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);				// Tamanho da estrutura em bytes

	hEventLeitura1 = CreateEvent(NULL, TRUE, FALSE, "Leitura1ON-OFF");
	hEventLeitura2 = CreateEvent(NULL, TRUE, FALSE, "Leitura2ON-OFF");
	hEventDetect = CreateEvent(NULL, TRUE, FALSE, "LeituraDetectoresON-OFF");
	hEventRetirada = CreateEvent(NULL, TRUE, FALSE, "RetiradaMensagensON-OFF");
	hsEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoDadosON-OFF");
	haEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoAlarmesON-OFF");
	hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EncerraTarefas");

	CreateProcess(
		"..\\Debug\\Railroad.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		//CREATE_NEW_CONSOLE,	
		NORMAL_PRIORITY_CLASS,	// CreationFlags
		NULL,	// lpEnvironment
		"..\\Railroad",
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


	do {
		printf("Tecle uma acao valida para gerar evento ou <Esc> para terminar\n");
		//espera uma tecla ser digitada

		nTecla = _getch();
		if (nTecla == '1') SetEvent(hEventLeitura1);	// Gera 1 evento
		if (nTecla == '2') SetEvent(hEventLeitura2); 	// Gera 1 evento
		if (nTecla == h)  SetEvent(hEventDetect); 	// Gera 1 evento
		if (nTecla == r) SetEvent(hEventRetirada);		// Gera 1 evento
		if (nTecla == s) SetEvent(hsEvent);		// Gera 1 evento
		if (nTecla == a) SetEvent(haEvent);		// Gera 1 evento
		else if (nTecla == ESC) SetEvent(hEscEvent);	// Termina processos
	} while (nTecla != ESC);

	WaitForMultipleObjects(4, EndProcess, TRUE, INFINITE);

	CloseHandle(hEventLeitura1);
	CloseHandle(hEventLeitura2);
	CloseHandle(hEventDetect);
	CloseHandle(hEventRetirada);
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

	printf("\nAcione uma tecla para terminar\n");
	_getch();

	return EXIT_SUCCESS;

}