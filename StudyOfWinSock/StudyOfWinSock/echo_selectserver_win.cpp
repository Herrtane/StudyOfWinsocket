// 0. �����ϱ� ����..
// �������� fd_set�� �������ʹ� �ٸ��� �ܼ��� ��Ʈ�� �迭�� �Ǿ����� �ʴ�.
// fd_count�� fd_array�� ����� ������ ����ü�� �Ǿ��ִ�.
// �ֳ��ϸ�, �������� ���� ��ũ���Ͱ� 0���� ���������� �����ϴ� ����������,
// �����쿡���� �ڵ��� 0���� ���������� �������� �ʰ�, �� ���̿� ��� ��Ģ���� �������� �ʴ´�.
// ���� ���� ���� �ΰ����� ������ �ʿ��� ���̴�.

// 1. select�� ���� ��Ƽ�÷��� ����� �帧
// 1) fd_set ���� �� select �Լ��� ���ڿ� �� ������ timeout ����
// 2) select() ȣ��
// 3) select() ���� return���� 1 �̻��̸� fd_set�� ��ȭ�� ����ٴ� �ǹ��̹Ƿ�, 'select() ���Ŀ��� ���� 1��' �ڵ��� ã�´�.

// 2. �ΰ����� �������
// : echo_server������ argc, argv�� Port�� �Է¹޾�����, �������ʹ� ���α׷� ���� �ÿ� ������ �Է¹޵��� ����

// 3. ������ ��
// : Ŭ���̾�Ʈ����, ip�ּҸ� ���� �߿� �Է¹޾Ƽ� �̸� �ּ� ������ �ְ� ������, �������� ���ڿ� �����ϴ� ����� �ʹ� ����������.
// ������ argv�� �Է¹޴°� ������ �ξ� ���ѵ�... �׷��� �ٸ� ���α׷������� Ip�ּҴ� conf ���Ͽ��� ������ �Է��� ���ϱ�?
// : �ذ�å ���� - C++�� string�� ����ؼ� c_str()���� ��ȯ�ϸ� char*�� ��ȯ�� �ȴ�. �� ������� Ŭ���̾�Ʈ�� �ذ��ߴ�!
// Ȥ�� sprintf�� ����ϴ� �����..? NULL ���ڵ� ���ԵǴϱ�.

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
using namespace std;

#define BUF_SIZE 1024
void ErrorHandling(const char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;												// handle of socket
	SOCKADDR_IN servAdr, clntAdr;
	TIMEVAL timeout;
	fd_set reads, cpyReads;

	int adrSz, strLen, fdNum, portNum;
	char buf[BUF_SIZE];
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	printf("Input port number:");
	cin >> portNum;

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(portNum);

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	FD_ZERO(&reads);															// fd_set ����
	FD_SET(hServSock, &reads);													// 

	while (1) {
		cpyReads = reads;
		timeout.tv_sec = 5;														// timeout ����
		timeout.tv_usec = 5000;													// select() ���� timeout�� �����ð����� ���ϹǷ�
																				// select() ���� �Ź� �ٽ� �ʱ�ȭ�ؾ��Ѵ�.

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR) {	// select()
			ErrorHandling("select() error");
			break;
		}

		if (fdNum == 0) {														// timeout
			printf("select() timeout!\n");
			continue;
		}

		for (int i = 0; i < reads.fd_count; i++) {
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {						// fd_set���� ��ȭ�� �ִ� �ڵ��� ã�´�
				if (reads.fd_array[i] == hServSock) {							// connection request
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					FD_SET(hClntSock, &reads);
					printf("connected client: %d \n", hClntSock);
				}
				else{															// read message
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen == 0) {											// close request (EOF)
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(cpyReads.fd_array[i]);
						printf("closed client: %d \n", cpyReads.fd_array[i]);
					}
					else
						send(reads.fd_array[i], buf, strLen, 0);				// echo
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(const char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}