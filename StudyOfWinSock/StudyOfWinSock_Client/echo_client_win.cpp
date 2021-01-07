#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <iostream>
#include <string>

#define BUF_SIZE 1024
using namespace std;

void ErrorHandling(const char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;									// handle of socket
	SOCKADDR_IN servAddr;

	char message[BUF_SIZE];
	int strLen, portNum;
	string ipNum;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);		// PF : protocol family
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	printf("Input server IP addr:");
	cin >> ipNum;
	printf("Input server port:");
	cin >> portNum;


	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;					// AF : address family
	servAddr.sin_addr.s_addr = inet_addr(ipNum.c_str());
	servAddr.sin_port = htons(portNum);				// host to network byte order
													// network byte order = big endian

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");
	else
		printf("Connected....................\n");

	while (1) {
		fputs("Input message(Q to quit) : ", stdout);
		fgets(message, BUF_SIZE, stdin);

		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		send(hSocket, message, strlen(message), 0);
		strLen = recv(hSocket, message, BUF_SIZE - 1, 0);
		message[strLen] = 0;
		printf("Message from server : %s \n", message);

	}
	closesocket(hSocket);
	WSACleanup();
	return 0;

}

void ErrorHandling(const char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
