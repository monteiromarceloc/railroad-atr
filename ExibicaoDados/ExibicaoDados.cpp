#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

//Impressão do cabeçalho de paginas
void cabecalho() {
	printf("\n     PAINEL DE EXIBICAO DADOS FERROVIARIOS   \n\n");
	printf("\n");
}

HANDLE hsEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoDadosON-OFF");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EncerraTarefas");


int main()
{
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[42];			//mensagem recebida do mailslot
	string Buffer, aux;				//Buffers de armazenamento

	HANDLE hEventDados;			//Handle para evento de sinalização de MailSlot
	HANDLE hEventMailslotDados;
	HANDLE hMailslotDados;
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

	HANDLE Events[2] = { hsEvent, hEscEvent };

	SetConsoleTitle(TEXT(" Dados Ferroviarios"));

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
			ResetEvent(hsEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hsEvent); }
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}

		GetMailslotInfo(hMailslotDados, &MaxMsgSize, &NextMsgSize, &MsgCont, &Timeout);

		if (MsgCont == 0) {}
		else if (MsgCont > 0) {

			bStatus = ReadFile(hMailslotDados, &MsgBuffer, 42, &dwBytesLidos, NULL);
			if (!bStatus)
				error = GetLastError();
			//Imprime as mensagens*/
			Buffer = MsgBuffer;
			aux = "";

			for (int i = 29; i < 41; i++) { cout << Buffer[i]; }
			cout << "  NSEQ: ";
			for (int i = 0; i < 6; i++) { cout << Buffer[i]; }
			cout << " REMOTA: ";
			for (int i = 11; i < 13; i++) { cout << Buffer[i]; }
			cout << " SENSOR: ";
			for (int i = 18; i < 26; i++) { cout << Buffer[i]; }
			cout << " ESTADO: ";
			for (int i = 27; i < 29; i++) { aux += Buffer[i]; }
			if (aux == "001")		cout << " Advertência para redução de velocidade";
			else if (aux == "100")	cout << " Término de precaução";
			else if (aux == "200")	cout << " Reassuma velocidade";
			else if (aux == "300")	cout << " Equipamento de grande porte na linha adjacente";
			else if (aux == "400")	cout << " Equipamento de infraestrutura próximo à Via";
			else if (aux == "500")	cout << " Linha impedida";
			else if (aux == "600")	cout << " Sinaleiro em pare";
			else if (aux == "700")	cout << " Buzine";
			else if (aux == "800")	cout << " Início de CTC";
			else if (aux == "900")	cout << " Fim de CTC";
			else if (aux == "1000")	cout << " Estação a 1km de distância";
			else if (aux == "1100")	cout << " Início de sinalização local";
			else if (aux == "1200")	cout << " Fim de sinalização local";
			else if (aux == "1300")	cout << " Sinaleiro em siga";
			else if (aux == "1400")	cout << " Manutenção mecânica";
			else if (aux == "1500")	cout << " Advertência de parada total";
			else if (aux == "1600")	cout << " Cruzamento com outra linha férrea";
			else if (aux == "1700")	cout << " Ponte a frente";
			else if (aux == "1800")	cout << " Túnel a frente";
			else if (aux == "1900")	cout << " Passagem de nível a 500m";
			cout << endl;
			posLeituraArquivo++;
			if (posLeituraArquivo == 200)
				posLeituraArquivo = 0;
		}
	} while (nTipoEvento != 1);

	//Destroi Handles
	CloseHandle(hEventDados);
	CloseHandle(hEventMailslotDados);
	CloseHandle(hMailslotDados);
	CloseHandle(Events);

	return EXIT_SUCCESS;
}

