#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"

vector<User> users;

HRESULT WINAPI serverWork(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer)
{
	IDirectPlay8Server *server;
	HRESULT result;
	DPNMSG_CREATE_PLAYER *newPlayer;
	DPN_PLAYER_INFO *playerDescription;
	DWORD dwSize;
	DPNHANDLE g_hSendTo;
	User user;
	if (((server = (IDirectPlay8Server*)pvUserContext)) == NULL)
		return E_FAIL;
	switch (dwMessageId)
	{
	case DPN_MSGID_CREATE_PLAYER:
		cout << "new player" << endl;
		newPlayer = (DPNMSG_CREATE_PLAYER*)pMsgBuffer;
		dwSize = 0;
		playerDescription = NULL;
		result = server->GetClientInfo(newPlayer->dpnidPlayer, playerDescription, &dwSize, 0);
		if (FAILED(result) && result != DPNERR_BUFFERTOOSMALL)
		{
			if (result == DPNERR_INVALIDPLAYER)
				break;
		}
		playerDescription = (DPN_PLAYER_INFO*)new BYTE[dwSize];
		ZeroMemory(playerDescription, sizeof(DPN_PLAYER_INFO));
		playerDescription->dwSize = sizeof(DPN_PLAYER_INFO);
		if (FAILED(server->GetClientInfo(newPlayer->dpnidPlayer, playerDescription, &dwSize, 0)))
		{
			delete[] playerDescription;
			break;
		}
		char name[32];
		wcstombs(name, playerDescription->pwszName, 32);

		user.id = newPlayer->dpnidPlayer;
		user.name = name;
		users.push_back(user);

		delete[] playerDescription;
		return S_OK;
	case DPN_MSGID_DESTROY_PLAYER:
		//need add to remove player from players
		break;
	case DPN_MSGID_RECEIVE:
		DPNMSG_RECEIVE* pReceive = (DPNMSG_RECEIVE*)pMsgBuffer;
		Message *message = (Message*)pReceive->pReceiveData;
		string userName;
		DPNID idSend = DPNID_ALL_PLAYERS_GROUP;
		for (User user : users)
		{
			if (user.id == pReceive->dpnidSender)
			{
				userName = user.name;
				break;
			}
		}
		int fl = 0;
		for (User user : users)
		{
			if (strcmp(message->name,"all") == 0)
			{
				fl = 1;
				break;
			}
			if (user.name == message->name)
			{
				idSend = user.id;
				fl = 1;
				break;
			}
		}
		
		DPN_BUFFER_DESC bufferSendDescription;
		Message messageToClient;
		if (fl == 0)
		{
			idSend = pReceive->dpnidSender;
			strcpy(messageToClient.data,"notUser");
			strcpy(messageToClient.name ,"server");
		}
		else
		{
			strcpy(messageToClient.data,message->data);
			strcpy(messageToClient.name,userName.c_str());
		}
		bufferSendDescription.dwBufferSize = sizeof(messageToClient);
		bufferSendDescription.pBufferData = (BYTE*)&messageToClient;
		result = server->SendTo(idSend, &bufferSendDescription, 1, 0, NULL, &g_hSendTo, DPNSEND_NOLOOPBACK);
		break;

	}
	return S_OK;
}

GUID APPGUID = { 0xede9493e, 0x6ac8, 0x4f15,{ 0x8d, 0x1, 0x8b, 0x16, 0x32, 0x0, 0xb9, 0x66 } };

Server::Server(int port, int maxPlayersSize, string sessionName)
{
	CoInitialize(NULL);
	ZeroMemory(&sessionDescription_, sizeof(DPN_APPLICATION_DESC));
	sessionDescription_.dwSize = sizeof(DPN_APPLICATION_DESC);
	WCHAR wSessionName[256];
	mbstowcs(wSessionName, sessionName.c_str(), sessionName.length() + 1);
	sessionDescription_.pwszSessionName = wSessionName;
	sessionDescription_.dwMaxPlayers = maxPlayersSize;
	sessionDescription_.guidApplication = APPGUID;
	sessionDescription_.dwFlags = DPNSESSION_CLIENT_SERVER;
	IDirectPlay8Address *address;
	result_ = CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC, IID_IDirectPlay8Address, (LPVOID *)&address);
	DWORD portServer = port;
	result_ = address->AddComponent(DPNA_KEY_PORT, &port, sizeof(DWORD), DPNA_DATATYPE_DWORD);
	result_ = address->SetSP(&CLSID_DP8SP_TCPIP);
	result_ = CoCreateInstance(CLSID_DirectPlay8Server, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Server, (LPVOID*)&server_);
	result_ = server_->Initialize(server_, serverWork, 0);
	result_ = server_->Host(&sessionDescription_, &address, 1, NULL, NULL, NULL, 0);
	result_ = address->Release();
}


Server::~Server()
{
	ZeroMemory(&sessionDescription_, sizeof(DPN_APPLICATION_DESC));
	result_ = server_->Release();
}
