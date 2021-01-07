// 0. Kernel Objects : OS�� ���ؼ� �����Ǵ� resource���� ������ �������� ������ ����ϱ� ���� ���������� ������ data block
// 1. main�Լ��� ȣ�� ��ü�� process�� �ƴ� thread�̴�.
// 2. handle�� ���ؼ� kernel object ������ �����ϰ�, kernel object�� ���ؼ� thread�� ������ ����
// => handle�� ���ؼ� thread�� ������ �� �ִ�.
// 3. kernel object�� ����Ǿ����� signaled state, ������� �ʾ����� non-signaled state
//		1) �� kernel object�� signaled���� Ȯ���ϱ� ���� WaitForSingleObject() ���
//		2) WaitForSingleObject() ȣ�� �� ��ȯ �Ŀ� kernel object�� �ٽ� non-signaled���°� �Ǵ� ���, auto-reset mode kernel object
//		3) �׷��� ���� ���, manual-reset mode kernel object
// 4. Windows OS�� ������(���α׷� ������)�� Dual-mode Operation
//		1) User mode : ���� ���α׷��� ���� ���
//		2) Kernel mode : OS�� ���� ���
// 5. User mode synchronization : ��� ��ȯ ���ʿ��ϹǷ� �ӵ��� ����
//		1) CRITICAL_SECTION ���
//	  Kernel mode synchronization : �����Ǵ� ����� ����, Deadlock ������ ���� Timeout ������ ����
//		1) Mutex Object ��� : Mutex�� �����Ǹ� non-signaled, auto-reset���� WaitForSingleObject()�� Mutex ���� ����
//		2) Semaphore Object ��� : Semaphore Value�� 0�̸� non-signaled, ���� auto-reset�̰� ���� ����.
//		3) Event Object ��� : auto�� manual-reset mode�� ������ �� �ִ�.

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include <iostream>

using namespace std;

#define BUF_SIZE 100
#define MAX_CLNT 50

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);
void ErrorHandling(const char* msg);

int clntCnt = 0;																// for multi client system
SOCKET clntSocks[MAX_CLNT];														//
HANDLE hMutex;

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;												// handle of socket
	SOCKADDR_IN servAdr, clntAdr;
	HANDLE hThread;																// handle of thread

	int adrSz, strLen, portNum;
	char buf[BUF_SIZE];
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	printf("Input port number:");
	cin >> portNum;

	hMutex = CreateMutex(NULL, FALSE, NULL);									// FALSE : create signaled Mutex

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(portNum);

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");


	while (1) {
		adrSz = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);

		WaitForSingleObject(hMutex, INFINITE);									// make Mutex non-signaled if Mutex is signaled
		clntSocks[clntCnt++] = hClntSock;
		ReleaseMutex(hMutex);													// make Mutex signaled

		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
		printf("connected client: %d \n", hClntSock);
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI HandleClnt(void* arg) {
	SOCKET hClntSock = *((SOCKET*)arg);
	int strLen = 0;
	char msg[BUF_SIZE];

	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)				// while client sends msg except EOF
		SendMsg(msg, strLen);

	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < clntCnt; i++) {											// remove disconnected client
		if (hClntSock == clntSocks[i]) {
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void SendMsg(char* msg, int len) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < clntCnt; i++)											// send to all clients
		send(clntSocks[i], msg, len, 0);
	ReleaseMutex(hMutex);
}

void ErrorHandling(const char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
