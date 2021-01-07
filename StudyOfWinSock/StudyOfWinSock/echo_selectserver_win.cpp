// 0. 시작하기 전에..
// 윈도우의 fd_set은 리눅스와는 다르게 단순히 비트의 배열로 되어있지 않다.
// fd_count와 fd_array를 멤버로 가지는 구조체로 되어있다.
// 왜냐하면, 리눅스는 파일 디스크립터가 0부터 순차적으로 증가하는 구조이지만,
// 윈도우에서는 핸들이 0부터 순차적으로 증가하지 않고, 그 사이에 어떠한 규칙성도 존재하지 않는다.
// 따라서 위와 같이 두가지의 정보가 필요한 것이다.

// 1. select에 의한 멀티플렉싱 기법의 흐름
// 1) fd_set 설정 및 select 함수의 인자에 들어갈 범위와 timeout 설정
// 2) select() 호출
// 3) select() 이후 return값이 1 이상이면 fd_set에 변화가 생겼다는 의미이므로, 'select() 이후에도 값이 1인' 핸들을 찾는다.

// 2. 부가적인 변경사항
// : echo_server까지는 argc, argv로 Port를 입력받았지만, 이제부터는 프로그램 실행 시에 별도로 입력받도록 변경

// 3. 연구할 점
// : 클라이언트에서, ip주소를 실행 중에 입력받아서 이를 주소 정보에 넣고 싶은데, 생각보다 문자열 조작하는 방식이 너무 복잡해진다.
// 기존의 argv로 입력받는게 오히려 훨씬 편한데... 그래서 다른 프로그램에서도 Ip주소는 conf 파일에서 별도로 입력한 것일까?
// : 해결책 연구 - C++의 string을 사용해서 c_str()으로 변환하면 char*로 변환이 된다. 이 방법으로 클라이언트를 해결했다!
// 혹은 sprintf를 사용하는 방법은..? NULL 문자도 삽입되니까.

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

	FD_ZERO(&reads);															// fd_set 설정
	FD_SET(hServSock, &reads);													// 

	while (1) {
		cpyReads = reads;
		timeout.tv_sec = 5;														// timeout 설정
		timeout.tv_usec = 5000;													// select() 이후 timeout은 남은시간으로 변하므로
																				// select() 전에 매번 다시 초기화해야한다.

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR) {	// select()
			ErrorHandling("select() error");
			break;
		}

		if (fdNum == 0) {														// timeout
			printf("select() timeout!\n");
			continue;
		}

		for (int i = 0; i < reads.fd_count; i++) {
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {						// fd_set에서 변화가 있는 핸들을 찾는다
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