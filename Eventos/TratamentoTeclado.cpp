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

#define d			0x64
#define p			0x70
#define o			0x6F
#define a			0x61
#define s			0x73
#define e			0x65
#define w			0x77
#define c			0x63
#define	ESC			0x1B

HANDLE hdEvent;			// Handle para Evento Comunicao de Dados on/off
HANDLE hpEvent;			// Handle para Evento Retirada de Dados on/off
HANDLE hoEvent;			// Handle para Evento Retirada de OPs on/off
HANDLE haEvent;			// Handle para Evento Retirada de Alarmes on/off
HANDLE hsEvent;			// Handle para Evento Exibicao de Dados on/off
HANDLE heEvent;			// Handle para Evento Exibicao de OPs on/off
HANDLE hwEvent;			// Handle para Evento Exibicao de Alarmes on/off
HANDLE hcEvent;			// Handle para Evento Limpeza tela de Alarmes
HANDLE hEscEvent;		// Handle para Evento Aborta

int main() {

	int nTecla;

	STARTUPINFO si;					// StartUpInformation para novo processo
	PROCESS_INFORMATION NewProcess, DadosProcess, OPProcess, AlarmeProcess;	// Informações sobre os novos processos criados
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);				// Tamanho da estrutura em bytes

	hdEvent = CreateEvent(NULL, TRUE, FALSE, "ComunicacaoDadosON-OFF");
	hpEvent = CreateEvent(NULL, TRUE, FALSE, "RetiradaDadosON-OFF");
	hoEvent = CreateEvent(NULL, TRUE, FALSE, "RetiradaOPsON-OFF");
	haEvent = CreateEvent(NULL, TRUE, FALSE, "RetiradaMensagensON-OFF");
	hsEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoDadosON-OFF");
	heEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoOPsON-OFF");
	hwEvent = CreateEvent(NULL, TRUE, FALSE, "ExibicaoMensagensON-OFF");
	hcEvent = CreateEvent(NULL, TRUE, FALSE, "ClearAlarmes");
	hEscEvent = CreateEvent(NULL, TRUE, FALSE, "EscEvento");

	CreateProcess(
		"..\\Debug\\Controle De Dados.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		//CREATE_NEW_CONSOLE,	
		NORMAL_PRIORITY_CLASS,	// CreationFlags
		NULL,	// lpEnvironment
		"..\\Controle de Dados",
		&si,			// lpStartUpInfo
		&NewProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de Dados.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,	
		NULL,	// lpEnvironment
		"..\\Exibicao de Dados",
		&si,			// lpStartUpInfo
		&DadosProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de OPs.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,	
		NULL,	// lpEnvironment
		"..\\Exibicao de Ops",
		&si,			// lpStartUpInfo
		&OPProcess);	// lpProcessInformation

	CreateProcess(
		"..\\Debug\\Exibicao de Alarmes.exe",
		NULL,	// linha de comando
		NULL,	// atributos de segurança: Processo
		NULL,	// atributos de segurança: Thread
		FALSE,	// herança de handles
		CREATE_NEW_CONSOLE,
		NULL,	// lpEnvironment
		"..\\Exibicao de Alarmes",
		&si,			// lpStartUpInfo
		&AlarmeProcess);	// lpProcessInformation

	HANDLE EndProcess[4] = { NewProcess.hProcess, DadosProcess.hProcess, OPProcess.hProcess, AlarmeProcess.hProcess };

	do {
		printf("Tecle uma acao valida para gerar evento ou <Esc> para terminar\n");
		nTecla = _getch();
		if (nTecla == d) SetEvent(hdEvent);			// ComunicacaoDadosON
		if (nTecla == p) SetEvent(hpEvent);			// RetiradaDadosON
		if (nTecla == o) SetEvent(hoEvent);			// RetiradaOPsON
		if (nTecla == a) SetEvent(haEvent);			// RetiradaAlarmesON
		if (nTecla == s) SetEvent(hsEvent);			// ExibicaoDadosON
		if (nTecla == e) SetEvent(heEvent);			// ExibicaoOPsON
		if (nTecla == w) SetEvent(hwEvent);			// ExibicaoAlarmesON
		if (nTecla == c) SetEvent(hcEvent);			// ClearAlarmes
		else if (nTecla == ESC) SetEvent(hEscEvent);	// EscEvento
	} while (nTecla != ESC);

	WaitForMultipleObjects(4, EndProcess, TRUE, INFINITE);
	
	CloseHandle(hdEvent);
	CloseHandle(hpEvent);
	CloseHandle(hoEvent);
	CloseHandle(haEvent);
	CloseHandle(hsEvent);
	CloseHandle(heEvent);
	CloseHandle(hwEvent);
	CloseHandle(hcEvent);
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