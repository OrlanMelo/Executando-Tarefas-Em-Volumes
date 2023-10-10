#include <Windows.h>
#include <ioapiset.h>
#include <string>
#include <iostream>
#pragma comment(lib,"kernel32.lib")

using namespace std;

class cFuncoes
{
private:

	HANDLE Identificador;

	TCHAR Volume[MAX_PATH];

public:

	void ObterIdentificadorDeUnidade(wstring Namespace)
	{
		Identificador = CreateFile(Namespace.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ |
			FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	}

	/*
	* 
	* Membro: "Letra" para obtermos o identificador do volume.
	* 
	*/
	void DesmontarVolume(wstring Letra)
	{
		if (Identificador == INVALID_HANDLE_VALUE)
			cout << "A unidade selecionada n�o foi encontrada.\n";
		else
		{

			//Ir� obter o identificador do volume associado a letra selecionada, para remontar mais tarde.
			if (GetVolumeNameForVolumeMountPoint(Letra.c_str(), Volume, MAX_PATH) == NULL)
				cout << "Ocorreu um erro durante a opera��o.." << GetLastError() << " \n";
			else
			{
				DWORD BytesRetornados;
				DeviceIoControl(Identificador, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &BytesRetornados, 0);
				DeviceIoControl(Identificador, IOCTL_DISK_UPDATE_PROPERTIES, 0, 0, 0, 0, &BytesRetornados, 0);

				cout << "O voluem ser� desbloqueado ap�s 15 segundos..\n";

				//Isto ir� manter o volume desmontado e inacess�vel durante 15 segundos.
				Sleep(15 * 1000);

				//O volume � liberado ao finalizar o identificador.
				CloseHandle(Identificador);
			}
		}
	}

	void AlterarStatusDeVolume(wstring Unidade,bool Offline)
	{
		ObterIdentificadorDeUnidade(Unidade);

		DWORD BytesRetornados;
		DeviceIoControl(Identificador, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &BytesRetornados, 0);

		if (Offline == true)
		{
			DeviceIoControl(Identificador, IOCTL_VOLUME_OFFLINE, 0, 0, 0, 0, &BytesRetornados, 0);
		}
		else
		{
			DeviceIoControl(Identificador, IOCTL_VOLUME_ONLINE, 0, 0, 0, 0, &BytesRetornados, 0);
		}

		CloseHandle(Identificador);
	}

	/*
	* 
	* Para que ocorra sucesso, n�o pode haver a letra selecionada j� atribu�da a outra unidade.
	* 
	*/
	void TrocarLetraDeUnidade(wstring Letra, wstring NovaLetra)
	{
		DeleteVolumeMountPoint(Letra.c_str());
		DefineDosDevice(DDD_RAW_TARGET_PATH, NovaLetra.c_str(), NovaLetra.c_str());
		DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE, NovaLetra.c_str(), NovaLetra.c_str());
		SetVolumeMountPoint(NovaLetra.c_str(), Volume);
	}

	void DeletarLetraDeUnidade(wstring Letra)
	{
		DeleteVolumeMountPoint(Letra.c_str());
	}

}Funcoes;

int main()
{

	cout << "O assistente est� efetuando altera��es no volume da unidade...\n\n";

	Funcoes.ObterIdentificadorDeUnidade(L"\\\\.\\D:");
	Funcoes.DesmontarVolume(L"D:\\");

	//Ap�s ter dado 15 segundos de espera da fun��o de desmontagem, a nova letra ser� atribu�da.
	cout << "Executando opera��es para a mudan�a de letra da unidade selecionada.\n";
	Funcoes.TrocarLetraDeUnidade(L"D:\\", L"E:\\");

	Sleep(2 * 1000);

	/*
	* Coloca o volume no estado offline.
	* Caso seja necess�rio deixar o volume indispon�vel por um tempo, colocar o volume offline � mais eficaz que apenas desmontar.
	* Ao colocar o volume em modo offline, ir� impedir que o mesmo seja montado novamente com facilidade.
	*/
	Funcoes.AlterarStatusDeVolume(L"\\\\.\\E:", true);

	Funcoes.AlterarStatusDeVolume(L"\\\\.\\E:", false);//Colocando o volume em modo online.

	Sleep(4 * 1000);//Ap�s 4 segundos, a letra da unidade ser� removida.
	Funcoes.DeletarLetraDeUnidade(L"E:\\");

	system("pause");
}