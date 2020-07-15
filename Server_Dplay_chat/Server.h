#pragma once
#include"dplay8.h"
#include"Structs.h"
#include<iostream>
#include<vector>

using namespace std;



class Server
{
private:
	HRESULT result_;
	IDirectPlay8Server *server_;
	DPN_APPLICATION_DESC sessionDescription_;
	
public:
	Server(int port, int maxPlayersSize,string sessionName);
	~Server();
};

