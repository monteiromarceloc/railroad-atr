#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

#define TAM_ALARM 30+5

void cabecalho() {
	printf("\n     SALA DE CONTROLE - ALERTAS   \n\n");
	printf("------------------------------------------------\n");
}


int main()
{
	/*-----------Declara��o de vari�veis internas------------*/

	char MsgBuffer[TAM_ALARM];	
	string Buffer;				

	HANDLE hEventAlarmes = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoAlarmesON-OFF");
	HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");
	HANDLE hEventDados;			
	HANDLE hEventMailslotDados;
	HANDLE hMailslot;
	BOOL bStatus;				//Status de recebimento
	DWORD error;
	DWORD dwBytesLidos;			//Numero de bytes recebidos
	DWORD ret;
	DWORD MaxMsgSize;
	DWORD NextMsgSize;
	DWORD MsgCont;
	DWORD Timeout;
	int nTipoEvento;

	HANDLE Events[2] = { hEventAlarmes, hEscEvent };

	SetConsoleTitle(TEXT("Console de Alertas"));

	//Cria��o de MailSlot servidor

	hEventDados = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarMailslotDados");
	hEventMailslotDados = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MailslotDadosEVENTO");

	WaitForSingleObject(hEventDados, INFINITE);

	hMailslot = CreateMailslot(
		"\\\\.\\mailslot\\MailslotDados",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	SetEvent(hEventMailslotDados);

	cabecalho();

	//Loop de recebimento
	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hEventAlarmes);
			//cout << "Tarefa de Exibição de Alarmes: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				ResetEvent(hEventAlarmes);
				//cout << "Tarefa de Exibição de Alarmes: ON" << endl;
			}
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslot, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);

		if (MsgCont == 0) {}
		else if (MsgCont > 0) {
			bStatus = ReadFile(hMailslot, &MsgBuffer, TAM_ALARM, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens
			for (int i = 22; i < 30; i++) { cout << MsgBuffer[i]; }
			cout << "  NSEQ: ";
			for (int i = 0; i < 7; i++) { cout << MsgBuffer[i]; }
			cout << " DETECTOR: ";
			for (int i = 11; i < 19; i++) { cout << MsgBuffer[i]; }
			if (MsgBuffer[20] == '1')
				cout << " RODA QUENTE DETECTADA ";
			cout << endl;
		}
	} while (nTipoEvento != 1);

	//Destroi Handles
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslot);
	CloseHandle(Events);
	CloseHandle(hEventAlarmes);

	return EXIT_SUCCESS;
}

