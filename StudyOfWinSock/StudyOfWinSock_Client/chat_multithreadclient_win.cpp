#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <string>

using namespace std;

#define BUF_SIZE 100
#define NAME_SIZE 20

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);
void ErrorHandling(const char* msg);

char name[NAME_SIZE] = "[DEFAULT]";								// for client name
char msg[BUF_SIZE];												// for client message

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;												// handle of socket
	SOCKADDR_IN servAddr;
	HANDLE hSendThread, hRecvThread;							// handle of thread

	int portNum;
	string ipNum, username;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);					// PF : protocol family
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	printf("Input server IP addr:");
	cin >> ipNum;
	printf("Input server port:");
	cin >> portNum;
	printf("Input your name:");
	cin >> username;

	sprintf(name, "[%s]", username.c_str());


	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;								// AF : address family
	servAddr.sin_addr.s_addr = inet_addr(ipNum.c_str());
	servAddr.sin_port = htons(portNum);							// host to network byte order
																// network byte order = big endian

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSocket, 0, NULL);
	hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSocket, 0, NULL);

	WaitForSingleObject(hSendThread, INFINITE);					// wait for finished thread 
	WaitForSingleObject(hRecvThread, INFINITE);					//
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

unsigned WINAPI SendMsg(void* arg) {
	SOCKET hSock = *((SOCKET*)arg);
	char nameMsg[NAME_SIZE+BUF_SIZE];							// Ex) [Name] Msg

	while (1) {
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
			closesocket(hSock);
			exit(0);
		}
		sprintf(nameMsg, "%s %s", name, msg);
		send(hSock, nameMsg, strlen(nameMsg),0);
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg) {
	SOCKET hSock = *((SOCKET*)arg);
	char nameMsg[NAME_SIZE+BUF_SIZE];

	int strLen;
	while (1) {
		strLen = recv(hSock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);
		if (strLen == -1)										// recv error
			return -1;
		nameMsg[strLen] = 0;
		fputs(nameMsg, stdout);
	}
	return 0;
}

void ErrorHandling(const char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
