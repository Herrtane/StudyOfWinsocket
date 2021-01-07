// 0. Kernel Objects : OS에 의해서 생성되는 resource들의 관리를 목적으로 정보를 기록하기 위해 내부적으로 생성한 data block
// 1. main함수의 호출 주체는 process가 아닌 thread이다.
// 2. handle을 통해서 kernel object 구분이 가능하고, kernel object를 통해서 thread의 구분이 가능
// => handle을 통해서 thread를 구분할 수 있다.
// 3. kernel object는 종료되었을때 signaled state, 종료되지 않았을때 non-signaled state
//		1) 이 kernel object가 signaled인지 확인하기 위해 WaitForSingleObject() 사용
//		2) WaitForSingleObject() 호출 뒤 반환 후에 kernel object가 다시 non-signaled상태가 되는 경우, auto-reset mode kernel object
//		3) 그렇지 않은 경우, manual-reset mode kernel object
// 4. Windows OS의 연산방식(프로그램 실행방식)은 Dual-mode Operation
//		1) User mode : 응용 프로그램의 실행 모드
//		2) Kernel mode : OS의 실행 모드
// 5. User mode synchronization : 모드 전환 불필요하므로 속도가 빠름
//		1) CRITICAL_SECTION 기반
//	  Kernel mode synchronization : 제공되는 기능이 많고, Deadlock 방지를 위한 Timeout 지정이 가능
//		1) Mutex Object 기반 : Mutex는 소유되면 non-signaled, auto-reset여서 WaitForSingleObject()로 Mutex 소유 가능
//		2) Semaphore Object 기반 : Semaphore Value이 0이면 non-signaled, 역시 auto-reset이고 위와 동일.
//		3) Event Object 기반 : auto와 manual-reset mode를 선택할 수 있다.

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
