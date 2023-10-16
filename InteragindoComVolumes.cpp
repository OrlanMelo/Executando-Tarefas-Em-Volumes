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
		{
			cout << "A unidade selecionada não foi encontrada.\n";
		}
		else
		{

			//Irá obter o identificador do volume associado a letra selecionada, para remontar mais tarde.
			if (GetVolumeNameForVolumeMountPoint(Letra.c_str(), Volume, MAX_PATH) == NULL)
			{
				cout << "Ocorreu um erro durante a operação.." << GetLastError() << " \n";
			}
			else
			{
				DWORD BytesRetornados;
				DeviceIoControl(Identificador, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &BytesRetornados, 0);
				DeviceIoControl(Identificador, IOCTL_DISK_UPDATE_PROPERTIES, 0, 0, 0, 0, &BytesRetornados, 0);

				cout << "O voluem será desbloqueado após 15 segundos..\n";

				//Isto irá manter o volume desmontado e inacessível durante 15 segundos.
				Sleep(15 * 1000);

				//O volume é liberado ao finalizar o identificador.
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
	* Para que ocorra sucesso, não pode haver a letra selecionada já atribuída a outra unidade.
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

	cout << "O assistente está efetuando alterações no volume da unidade...\n\n";

	Funcoes.ObterIdentificadorDeUnidade(L"\\\\.\\D:");
	Funcoes.DesmontarVolume(L"D:\\");

	//Após ter dado 15 segundos de espera da função de desmontagem, a nova letra será atribuída.
	cout << "Executando operações para a mudança de letra da unidade selecionada.\n";
	Funcoes.TrocarLetraDeUnidade(L"D:\\", L"E:\\");

	Sleep(2 * 1000);

	/*
	* Coloca o volume no estado offline.
	* Caso seja necessário deixar o volume indisponível por um tempo, colocar o volume offline é mais eficaz que apenas desmontar.
	* Ao colocar o volume em modo offline, irá impedir que o mesmo seja montado novamente com facilidade.
	*/
	Funcoes.AlterarStatusDeVolume(L"\\\\.\\E:", true);

	Funcoes.AlterarStatusDeVolume(L"\\\\.\\E:", false);//Colocando o volume em modo online.

	Sleep(4 * 1000);//Após 4 segundos, a letra da unidade será removida.
	Funcoes.DeletarLetraDeUnidade(L"E:\\");

	system("pause");
}
