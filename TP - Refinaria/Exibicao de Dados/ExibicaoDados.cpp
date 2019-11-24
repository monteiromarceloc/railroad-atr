#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>


using namespace std;

//Impressão do cabeçalho de paginas
void cabecalho() {
	printf("\n     PAINEL DE EXIBICAO DADOS DE PROCESSO   \n\n");
	printf("------------------------------------------------\n");

}//cabecalho*/

HANDLE hsEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoDadosON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

int main()
{
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[41];			//msg a receber pelo maislots
	string Buffer;				//Buffers de armazenamento
	
	HANDLE hEventDados;			//Handle para evento de sinalização de MailSlot
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

	HANDLE Events[2] = {hsEvent, hEscEvent};

	SetConsoleTitle(TEXT("Console de Dados de Processo"));

	//Criação de MailSlot servidor

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
			ResetEvent(hsEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hsEvent); }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslot, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);
		
		if (MsgCont == 0) {}
		else if (MsgCont > 0) {
			bStatus = ReadFile(hMailslot, &MsgBuffer, 41, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens
			Buffer = MsgBuffer;
			cout << Buffer << endl;
			/*
			for (int i = 32; i < 40; i++) { cout << Buffer[i]; }
			cout << " NSEQ: ";
			for (int i = 3; i < 9; i++) { cout << Buffer[i]; }
			cout << "  TEMP: ";
			for (int i = 10; i < 16; i++) { cout << Buffer[i]; }
			cout << " C  PRE: ";
			for (int i = 17; i < 21; i++) { cout << Buffer[i]; }
			cout << " kgf/m2  VOL: ";
			for (int i = 22; i < 28; i++) { cout << Buffer[i]; }
			cout << " m3  PROD: ";
			for (int i = 29; i < 31; i++) { cout << Buffer[i]; }
			cout << endl;
			*/
		}
	} while (nTipoEvento != 1);

	//Destroi Handles
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslot);
	CloseHandle(Events);

	return EXIT_SUCCESS;
}	

