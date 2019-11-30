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

}



int main() {

	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[TAM_MSG];			
	string Buffer, aux;				
	LONG lOldValue;

	HANDLE hEventExibeDados = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoMensagensON-OFF");
	HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");
	HANDLE Events[2] = { hEventExibeDados, hEscEvent };

	HANDLE hEventAlarme = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CriarArquivoAlarme");
	HANDLE hAlarme = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ArquivoCriado");		
	HANDLE hArqMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "MutexArquivo");
	HANDLE semaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaforoArquivoCheio");
	HANDLE hsemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaforoArquivoLer");
	HANDLE Events2[2] = { hsemaphore, hEscEvent };

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
		ret = WaitForMultipleObjects(2, Events, FALSE, 200);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hEventExibeDados);
			//cout << "Tarefa de Exibição de Dados: OFF" << endl;
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				ResetEvent(hEventExibeDados);
				//cout << "Tarefa de Exibição de Alarmes: ON" << endl;
			}
			else if (nTipoEvento == 1)	break;
		}
		else if (nTipoEvento == 1) {
			break;
		}
		
		ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {}
		else if (nTipoEvento == 1) { break; }
		
		WaitForSingleObject(hArqMutex, INFINITE);
		SetFilePointer(hFile, (posLeituraArquivo * TAM_MSG), NULL, FILE_BEGIN);
		bStatus = ReadFile(hFile, MsgBuffer, TAM_MSG, &dwBytesLidos, NULL);
		if (!bStatus)
			error = GetLastError();
		ReleaseSemaphore(semaphore, 1, &lOldValue);
		ReleaseMutex(hArqMutex);
		aux = "";

		for (int i = 29; i < 38; i++) { cout << MsgBuffer[i]; }
		cout << "  NSEQ: ";
		for (int i = 0; i < 7; i++) { cout << MsgBuffer[i]; }
		cout << " REMOTA: ";
		for (int i = 11; i < 13; i++) { cout << MsgBuffer[i]; }
		cout << " SENSOR: ";
		for (int i = 18; i < 26; i++) {
			cout << MsgBuffer[i];
			aux += MsgBuffer[i];
		}
		cout << " ESTADO: ";
		if (aux == "QNQ-0655")		cout << " Advertencia para reducaoo de velocidade";
		else if (aux == "DXO-0173")	cout << " Termino de precacao";
		else if (aux == "HMS-0087")	cout << " Reassuma velocidade";
		else if (aux == "TBV-0423")	cout << " Equipamento de grande porte na linha adjacente";
		else if (aux == "FLD-0036")	cout << " Equipamento de infraestrutura proximo a Via";
		else if (aux == "EFX-0867")	cout << " Linha impedida";
		else if (aux == "NAY-0807")	cout << " Sinaleiro em pare";
		else if (aux == "DIH-0278")	cout << " Buzine";
		else if (aux == "CDW-0945")	cout << " Inicio de CTC";
		else if (aux == "XGC-0651")	cout << " Fim de CTC";
		else if (aux == "HAE-0193")	cout << " Estacao a 1km de distancia";
		else if (aux == "URK-0007")	cout << " Inicio de sinalizacao local";
		else if (aux == "CAU-0759")	cout << " Fim de sinalizacao local";
		else if (aux == "PCU-0507")	cout << " Sinaleiro em siga";
		else if (aux == "VNO-0335")	cout << " Manutencao mecanica";
		else if (aux == "OJS-0588")	cout << " Advertencia de parada total";
		else if (aux == "CRS-0608")	cout << " Cruzamento com outra linha ferrea";
		else if (aux == "SSW-0828")	cout << " Ponte a frente";
		else if (aux == "UNW-0954")	cout << " Tunel a frente";
		else if (aux == "ICX-0118")	cout << " Passagem de nivel a 500m";
		else cout << " ok";
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
	CloseHandle(hAlarme);
	CloseHandle(hArqMutex);
	CloseHandle(hEventExibeDados);
	CloseHandle(hEscEvent);
	CloseHandle(hEventAlarme);

	return EXIT_SUCCESS;
}