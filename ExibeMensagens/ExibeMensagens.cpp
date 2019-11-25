#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>

#define TAM_MSG 41+6

using namespace std;

void cabecalho() {
	printf("\n     SALA DE CONTROLE - MENSAGENS   \n\n");
	printf("------------------------------------------------\n");

}//cabecalho*/


HANDLE hwEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoMensagensON-OFF");
HANDLE hcEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ClearMensagens");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

int main() {
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[TAM_MSG];			//msg a receber pelo maislots
	string Buffer, aux;				//Buffers de armazenamento
	LONG lOldValue;

	HANDLE Events[3] = { hwEvent, hEscEvent, hcEvent };

	HANDLE hEventAlarme = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarArquivoAlarme");		// Handle para sinalização de criacao de arquivo
	HANDLE hAlarme = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ArquivoCriado");				// Sinalizacao: arquivo criado
	HANDLE hArqMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "MutexArquivo");
	HANDLE semaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaforoArquivoCheio");
	HANDLE hsemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaforoArquivoLer");
	HANDLE Events2[3] = { hsemaphore, hEscEvent, hcEvent };

	BOOL bStatus;				//Status de recebimento
	DWORD error;
	DWORD dwBytesLidos;			//Numero de bytes recebidos
	DWORD ret;
	int nTipoEvento;
	int posLeituraArquivo = 0;

	SetConsoleTitle(TEXT("Console de Mensagens"));

	WaitForSingleObject(hEventAlarme, INFINITE);
	HANDLE hFile = CreateFile("..\\Files\\Mensagens.arq",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,		// abre para leitura e escrita
		NULL,									// atributos de segurança
		CREATE_ALWAYS,							// cria novo arquivo
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	SetEvent(hAlarme);

	cabecalho();

	do {
		ret = WaitForMultipleObjects(3, Events, FALSE, 200);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hwEvent);
			//cout << "Tarefa de Exibição de Dados: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				ResetEvent(hwEvent);
				//cout << "Tarefa de Exibição de Alarmes: ON" << endl;
			}
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}
		else if (nTipoEvento == 2) {
			ResetEvent(hcEvent);
			system("cls");
		}
		ret = WaitForMultipleObjects(3, Events2, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {}
		else if (nTipoEvento == 1) { break; }
		else if (nTipoEvento == 2) {
			ResetEvent(hcEvent);
			system("cls");
		}
		WaitForSingleObject(hArqMutex, INFINITE);
		SetFilePointer(hFile, (posLeituraArquivo * TAM_MSG), NULL, FILE_BEGIN);
		bStatus = ReadFile(hFile, MsgBuffer, TAM_MSG, &dwBytesLidos, NULL);
		if (!bStatus)
			error = GetLastError();
		ReleaseSemaphore(semaphore, 1, &lOldValue);
		ReleaseMutex(hArqMutex);

		//Buffer = MsgBuffer;
		//cout << Buffer << endl;

		aux = "";

		for (int i = 29; i < 41; i++) { cout << MsgBuffer[i]; }
		cout << "  NSEQ: ";
		for (int i = 0; i < 6; i++) { cout << MsgBuffer[i]; }
		cout << " REMOTA: ";
		for (int i = 11; i < 13; i++) { cout << MsgBuffer[i]; }
		cout << " SENSOR: ";
		for (int i = 18; i < 26; i++) {
			cout << MsgBuffer[i];
			aux += MsgBuffer[i];
		}
		cout << " ESTADO: ";
		if (aux == "00000001")		cout << " Advert�ncia para redu��o de velocidade";
		else if (aux == "00000100")	cout << " Termino de precacao";
		else if (aux == "00000200")	cout << " Reassuma velocidade";
		else if (aux == "00000300")	cout << " Equipamento de grande porte na linha adjacente";
		else if (aux == "00000400")	cout << " Equipamento de infraestrutura proximo a Via";
		else if (aux == "00000500")	cout << " Linha impedida";
		else if (aux == "00000600")	cout << " Sinaleiro em pare";
		else if (aux == "00000700")	cout << " Buzine";
		else if (aux == "00000800")	cout << " Inicio de CTC";
		else if (aux == "00000900")	cout << " Fim de CTC";
		else if (aux == "00001000")	cout << " Estacao a 1km de distancia";
		else if (aux == "00001100")	cout << " Inicio de sinalizacao local";
		else if (aux == "00001200")	cout << " Fim de sinalizacao local";
		else if (aux == "00001300")	cout << " Sinaleiro em siga";
		else if (aux == "00001400")	cout << " Manutencao mecanica";
		else if (aux == "00001500")	cout << " Advertencia de parada total";
		else if (aux == "00001600")	cout << " Cruzamento com outra linha ferrea";
		else if (aux == "00001700")	cout << " Ponte a frente";
		else if (aux == "00001800")	cout << " Tunel a frente";
		else if (aux == "00001900")	cout << " Passagem de nivel a 500m";
		cout << endl;

		posLeituraArquivo++;
		if (posLeituraArquivo == 500)
			posLeituraArquivo = 0;

	} while (nTipoEvento != 1);

	//Destroi Handles
	CloseHandle(Events);
	CloseHandle(Events2);
	CloseHandle(hsemaphore);
	CloseHandle(semaphore);
	CloseHandle(Events);
	CloseHandle(hEventAlarme);
	CloseHandle(hAlarme);
	CloseHandle(hArqMutex);

	return EXIT_SUCCESS;
}