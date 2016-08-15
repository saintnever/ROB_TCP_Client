#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>



// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
//#define UP_UP 0x01
//#define DOWN_UP 0x02
//#define LEFT_UP 0x03
//#define RIGHT_UP 0x04
//#define UP_DOWN 0x11
//#define DOWN_DOWN 0x12
//#define LEFT_DOWN 0x13
//#define RIGHT_DOWN 0x14

#define UP_UP  "uu"
#define DOWN_UP "du"
#define LEFT_UP "lu"
#define RIGHT_UP "ru"
#define UP_DOWN "ud"
#define DOWN_DOWN "dd"
#define LEFT_DOWN "ld"
#define RIGHT_DOWN "rd"

HHOOK hKeyHook;
KBDLLHOOKSTRUCT kbdStruct;
SOCKET ConnectSocket = INVALID_SOCKET;
char* sendbuf;
LRESULT CALLBACK KeyboardProc(UINT, WPARAM, LPARAM);
int TCPstart(char*);
int TCPsend(char*);
int TCPclosesend();
int TCPreceive();
int TCPcleanup();


int __cdecl main(int argc, char **argv) {
//int main(){
	/* input server ip address
	char* server_addr="127.0.0.1";
	cout<<"Please input IP address"<<endl;*/
	//cin >> server_addr;
	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}
	//Establish connection, return when there is error
	if (TCPstart(argv[1])) {
		return 1;
	}
	cout << "TCP connection established" << endl;
	sendbuf = "test";
	TCPsend(sendbuf);
	//send(ConnectSocket, sendbuf, strlen(sendbuf), 0);

	//Keyboard logger setup
	HINSTANCE hInst = NULL;
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc, hInst, 0);

	/** Dispatch message */
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		TCPsend(sendbuf);

	}
	//cleanup
	TCPcleanup();
    UnhookWindowsHookEx(hKeyHook);
	return 0;
}

LRESULT CALLBACK KeyboardProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
	if (uMsg >= HC_ACTION) {	
		switch (wParam)
		{
		case WM_KEYDOWN:
			switch ((unsigned int)kbdStruct.vkCode)
			{
			case VK_LEFT:
				printf("left key pressed down\n");
				sendbuf = LEFT_DOWN;
				break;
			case VK_RIGHT:
				printf("right key pressed down\n");
				sendbuf = RIGHT_DOWN;
				break;
			case VK_UP:
				printf("Up key pressed down\n");
				sendbuf = UP_DOWN;
				break;
			case VK_DOWN:
				printf("Down key pressed down\n");
				sendbuf = DOWN_DOWN;
				break;
			default:
				printf("in keydown msg\n");
				break;
			}
			//TCPsend(sendbuf, ConnectSocket);
			break;
		case WM_KEYUP:
			switch ((unsigned int)kbdStruct.vkCode)
			{
			case VK_LEFT:
				printf("left key released\n");
				sendbuf = LEFT_UP;
				break;
			case VK_RIGHT:
				printf("right key released\n");
				sendbuf = RIGHT_UP;
				break;
			case VK_UP:
				printf("Up key released\n");
				sendbuf = UP_UP;
				break;
			case VK_DOWN:
				printf("Down keys released\n");
				sendbuf = DOWN_UP;
				break;
			default:
				printf("in keyup msg\n");
				break;
			}
			//TCPsend(sendbuf, ConnectSocket);
			break;
		default:
			//printf("in other msg%d, key %d\n", uMsg, wParam);
			return DefWindowProc(NULL, uMsg, wParam, lParam);
		}
	}
	return CallNextHookEx(hKeyHook, uMsg, wParam, lParam);
}

int TCPstart(char* addr) {
	WSADATA wsaData;
	int iResult;
	//init winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult!=0)
	{
	printf("WSAStartup failed: %d\n", iResult);
	}
	cout << "wsastartup successful" << endl;
	//create socket
	struct addrinfo  *result = NULL,
	*ptr = NULL,
	hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//resolve server address and port
	iResult = getaddrinfo(addr, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
	printf("getaddrinfo failed:%d\n", iResult);
	WSACleanup();
	return iResult;
	}
	//attempt to connect to first address returned by getaddrinfo
	ptr = result;
	//Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
	printf("Error at socket(): %d\n", WSAGetLastError());
	freeaddrinfo(result);
	WSACleanup();
	return iResult;
	}
	//establish connection to a server
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult==SOCKET_ERROR)
	{
	closesocket(ConnectSocket);
	ConnectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (ConnectSocket==INVALID_SOCKET)
	{
	printf("Unable to connect to server!\n");
	WSACleanup();
	return iResult;
	}
	return iResult;
}

int TCPsend(char* data) {
	int iResult;
	//send an initial buffer
	//iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	iResult = send(ConnectSocket, data, strlen(data), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return iResult;
	}
	printf("Bytes Sent: %ld\n", iResult);
	return iResult;
}

int TCPsendclose() {
	//shutdown the connection for sending since all data is sent
	//the client can still use the connectsocket for receiving data
	int iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int TCPreceive() {
	//sending and receiving data
	int recvbuflen = DEFAULT_BUFLEN;
//	char sendbuf[20];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;

	//receive data until server closes connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult>0)
		{
			printf("Bytes received: %d\n", iResult);
		}
		else if (iResult == 0)
		{
			printf("Connection closed\n");
		}
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);

	return iResult;
}

int TCPcleanup() {
	//cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}