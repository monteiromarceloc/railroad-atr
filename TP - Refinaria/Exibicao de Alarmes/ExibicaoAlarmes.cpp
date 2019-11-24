#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>

#define TAM_ALARM 30

using namespace std; 

void cabecalho() {
	printf("\n     PAINEL DE EXIBICAO ALARMES   \n\n");
	printf("------------------------------------------------\n");

}//cabecalho*/


HANDLE hwEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ExibicaoAlarmesON-OFF");
HANDLE hcEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ClearAlarmes");
HANDLE hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "EscEvento");

int main(){
	/*-----------Declaração de variáveis internas------------*/

	char MsgBuffer[TAM_ALARM];			//msg a receber pelo maislots
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

	SetConsoleTitle(TEXT("Console de Alarmes"));

	WaitForSingleObject(hEventAlarme, INFINITE);
	HANDLE hFile = CreateFile("..\\Alarme\\Alarmes.arq",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,		// abre para leitura e escrita
		NULL,									// atributos de segurança
		CREATE_ALWAYS,							// cria novo arquivo
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	SetEvent(hAlarme);

	cabecalho();

	do {
		ret = WaitForMultipleObjects(3, Events, FALSE, 500);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ResetEvent(hwEvent);
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) { ResetEvent(hwEvent); }
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
			if(nTipoEvento == 0){}
			else if (nTipoEvento == 1) { break; }
			else if (nTipoEvento == 2) {
				ResetEvent(hcEvent);
				system("cls");
			}
		WaitForSingleObject(hArqMutex, INFINITE);
		SetFilePointer(hFile, (posLeituraArquivo * TAM_ALARM), NULL, FILE_BEGIN);
		bStatus = ReadFile(hFile, MsgBuffer, TAM_ALARM, &dwBytesLidos, NULL);
		if (!bStatus)
			error = GetLastError();
		ReleaseSemaphore(semaphore, 1, &lOldValue);
		ReleaseMutex(hArqMutex);

		Buffer = MsgBuffer;
		cout << Buffer << endl;
		/*
		aux = "";
		
		for (int i = 17; i < 25; i++) { cout << Buffer[i]; }
		cout << "  NSEQ: ";
		for (int i = 3; i < 9; i++) { cout << Buffer[i]; }
		for (int i = 13; i < 16; i++) { aux += Buffer[i]; }
		if (aux == "001")	cout << " Fissura na tubulacao.";
		else if (aux == "100")	cout << " Ruptura no revestimento.";
		else if (aux == "200")	cout << " Incendio de Jato - JetFire.";
		else if (aux == "300")	cout << " BLEVE.";
		else if (aux == "400")	cout << " FlashFire.";
		else if (aux == "500")	cout << " Explosao em Nuvem.";
		else if (aux == "600")	cout << " Dispersao Passiva de Nuvem.";
		else if (aux == "700")	cout << " Vazamento de Liquido no Solo";
		else if (aux == "800")	cout << " Dispersao de Gas Denso.";
		else if (aux == "900")	cout << " Radiacao Termica.";
		cout << " PRI: ";
		for (int i = 10; i < 12; i++) { cout << Buffer[i]; }
		*/

		posLeituraArquivo++;
		if (posLeituraArquivo == 200)
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