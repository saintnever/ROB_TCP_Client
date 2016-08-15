/*//A TCP client to connect to tablet robot ROB
#include<WinSock2.h>

SOCKET s; //socket handle

//CONNECTTOHOST 
bool ConnectToHost(int PortNo, char* IPAddress) {
	//start winsock
	WSADATA wsadata;
	int error = WSAStartup(0x0202, &wsadata);
	if (error)
		return false;
	if (wsadata.wVersion != 0x0202)
	{
		WSACleanup();
		return false;
	}
	// fill out information needed to init a socket
	SOCKADDR_IN target;
	target.sin_family = AF_INET; //address family Internet
	target.sin_port = htons(PortNo);
	target.sin_addr.s_addr = inet_addr(IPAddress); //target IP

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s==INVALID_SOCKET)
	{
		return false;	
	}

	//try connecting
	if (connect(s, (SOCKADDR*)&target, sizeof(target)) == SOCKET_ERROR)
	{
		return false;
	}
	else
		return true;
}

//CLOSECONNECTION - shutdown the socket and closes any connection on it
void CloseConnect() {
	if (s)
		closesocket(s);
	WSACleanup();
}
*/