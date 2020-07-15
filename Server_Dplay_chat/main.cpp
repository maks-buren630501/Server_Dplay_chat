#include<iostream>
#include"Server.h"

using namespace std;

int main()
{
	Server *server = new Server(8087, 10, "mySession");
	while (true)
	{

	}
	return 0;
}