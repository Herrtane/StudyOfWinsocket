#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
void ErrorHandling(const char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;									// handle of socket
	SOCKADDR_IN servAddr;

	char message[30];
	int strLen;
	if (argc != 3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);		// PF : protocol family
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;					// AF : address family
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));		// host to network byte order
													// network byte order = big endian

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	strLen = recv(hSocket, message, sizeof(message) - 1, 0);
	if (strLen == -1)
		ErrorHandling("read() error!");
	printf("Message from server : %s \n", message);
	closesocket(hSocket);
	WSACleanup();
	return 0;

}

void ErrorHandling(const char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}