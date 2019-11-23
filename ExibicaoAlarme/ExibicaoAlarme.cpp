#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

//Impressão do cabeçalho de paginas
void cabecalho() {
	printf("\n     PAINEL DE EXIBICAO ALARMES   \n\n");
	printf("\n");
}

HANDLE haEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoAlarmesON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");


int main()
{
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[42], MsgBuffer2[30];			//mensagem recebida do mailslot
	string Buffer, Buffer2;				//Buffers de armazenamento para os avisos e detectores

	HANDLE hEventDados;			//Handle para evento de sinalização de MailSlot
	HANDLE hEventMailslotDados;
	HANDLE hMailslotDados;
	HANDLE hEventDetectores;			//Handle para evento de sinalização de MailSlot
	HANDLE hEventMailslotDetectores;
	HANDLE hMailslotDetectores;
	DWORD error;
	DWORD MaxMsgSize;
	DWORD NextMsgSize;
	DWORD MsgCont;
	DWORD Timeout;
	BOOL bStatus;
	DWORD dwBytesLidos;			//Numero de bytes recebidos
	DWORD ret;
	int nTipoEvento;
	int posLeituraArquivo = 0;

	HANDLE Events[2] = { haEvent, hEscEvent };

	SetConsoleTitle(TEXT("Alarmes"));

	hEventDados = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarMailslotDados");
	hEventMailslotDados = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MailslotDados");

	WaitForSingleObject(hEventDados, INFINITE);

	hMailslotDados = CreateMailslot(
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
			ResetEvent(haEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(haEvent); }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslotDados, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);

		if (MsgCont == 0) {}
		else if (MsgCont > 0) 
		{

			bStatus = ReadFile(hMailslotDados, &MsgBuffer, 42, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens*/
			Buffer = MsgBuffer;
			

			for (int i = 29; i < 41; i++) { cout << Buffer[i]; }
			cout << "  NSEQ: ";
			for (int i = 0; i < 6; i++) { cout << Buffer[i]; }
			cout << " REMOTA: ";
			for (int i = 11; i < 13; i++) { cout << Buffer[i]; }
			cout << " FALHA DE HARDWARE ";
			
		}
	} while (nTipoEvento != 1);

	//receber dados detectores
	hEventDetectores = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarMailslotDetectores");
	hEventMailslotDetectores = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MailslotDetectores");

	WaitForSingleObject(hEventDetectores, INFINITE);

	hMailslotDetectores = CreateMailslot(
		"\\\\.\\mailslot\\MailslotDetectores",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	SetEvent(hEventMailslotDetectores);

	cabecalho();

	//Loop de recebimento
	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(haEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(haEvent); }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslotDetectores, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);

		if (MsgCont == 0) {}
		else if (MsgCont > 0)
		{

			bStatus = ReadFile(hMailslotDetectores, &MsgBuffer2, 30, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens*/
			Buffer2 = MsgBuffer2;
			

			for (int i = 22; i < 29; i++) { cout << Buffer2[i]; }
			cout << "  NSEQ: ";
			for (int i = 0; i < 8; i++) { cout << Buffer2[i]; }
			cout << " DETECTOR: ";
			for (int i = 11; i < 19; i++) { cout << Buffer2[i]; }
			cout << " RODA QUENTE DETECTADA ";

		}
	} while (nTipoEvento != 1);


	//Destroi Handles
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslotDados);
	CloseHandle(Events);
	CloseHandle(hEventDetectores);		
	CloseHandle(hEventMailslotDetectores);
	CloseHandle(hMailslotDetectores);

	return EXIT_SUCCESS;
}