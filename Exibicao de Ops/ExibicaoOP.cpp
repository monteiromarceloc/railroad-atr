#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

//Impressão do cabeçalho de paginas
void cabecalho() {
	printf("\n     PAINEL DE EXIBICAO ORDENS DE PRODUCAO   \n\n");
	printf("------------------------------------------------\n");
}

HANDLE heEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoOPsON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

int main()
{
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[35];			//mensagem recebida do mailslot
	string Buffer;				//Buffers de armazenamento

	HANDLE hEventOP;			//Handle para evento de sinalização de MailSlot
	HANDLE hEventMailslotOP;
	HANDLE hMailslotOP;
	DWORD error;
	DWORD MaxMsgSize;
	DWORD NextMsgSize;
	DWORD MsgCont;
	DWORD Timeout;
	BOOL bStatus;
	DWORD dwBytesLidos;			//Numero de bytes recebidos
	DWORD ret;
	int nTipoEvento;

	HANDLE Events[2] = { heEvent, hEscEvent };

	SetConsoleTitle(TEXT("Console de Ordens de Producao"));

	hEventOP = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarMailslotOP");
	hEventMailslotOP = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MailslotOPEVENTO");

	WaitForSingleObject(hEventOP, INFINITE);

	hMailslotOP = CreateMailslot(
		"\\\\.\\mailslot\\MailslotOP",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	SetEvent(hEventMailslotOP);

	cabecalho();

	//Loop de recebimento
	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(heEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(heEvent); }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslotOP, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);

		if (MsgCont == 0) {}
		else if (MsgCont > 0) {

			bStatus = ReadFile(hMailslotOP, &MsgBuffer, 35, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens*/
			Buffer = MsgBuffer;
			cout << "NSEQ: ";
			for (int i = 3; i < 9; i++) { cout << Buffer[i]; }
			cout << " NUMERO DA OP: ";
			for (int i = 10; i < 18; i++) { cout << Buffer[i]; }
			cout << " SETOR: ";
			for (int i = 19; i < 21; i++) { cout << Buffer[i]; }
			cout << " RECEITA: ";
			for (int i = 22; i < 25; i++) { cout << Buffer[i]; }
			cout << " INICIO PREVISTO: ";
			for (int i = 26; i < 34; i++) { cout << Buffer[i]; }
			cout << endl;
		}
	} while (nTipoEvento != 1);

	//Destroi Handles
	CloseHandle(hEventOP);
	CloseHandle(hEventMailslotOP);
	CloseHandle(hMailslotOP);
	CloseHandle(Events);

	return EXIT_SUCCESS;
}

