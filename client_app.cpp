#include <stdio.h> 
#include <string.h>    
#include <string>

#include <iostream>

#pragma comment(lib,"Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

bool is_num(const std::string& msg)
{
	for (int i=0; i<4; ++i)
		if (!(msg[0] > '0' && msg[0] <= '9')) return false;
	for (int i = 0; i < 3; ++i)
		for (int j = i + 1; j < 4; ++j)
			if (msg[i] == msg[j]) return false;
	return true;
}
bool is_stop(const std::string& msg)
{
	return msg[0] == 's' && msg[1] == 't' && msg[2] == 'o' && msg[3] == 'p';
}
bool wrong(const std::string& msg)
{
	if (msg.size() < 4) return false;
	return is_num(msg) || is_stop(msg);
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Сокет не удалось создать.");
		return -1;
	}




	sockaddr_in srv;
	memset(&srv, 0, sizeof(srv));
	const char* ip_str ="127.0.0.1";
	srv.sin_family = AF_INET; 
	inet_pton(AF_INET, ip_str, &srv.sin_addr.s_addr);
	srv.sin_port = htons(5555); 

	if (connect(s, (struct sockaddr*)&srv, sizeof(srv)) < 0)
	{
		printf("can't connect to the server\n");
		return -1;
	}

	char msg[1000];
	int msg_len = strlen(msg); 
	
	int ret;
	bool gotit = false;
	std::string str;
	char reply[2000];
	while ((ret = recv(s, reply, 2000, 0)) > 0)
	{
		gotit = true;
		for (int i = 0; i < ret; ++i)
			printf("%c", reply[i]);
		printf("\n");
		if (ret >= 10 && reply[0]=='s')
		{
			closesocket(s);
			WSACleanup();
			return 0;
		}
	A:std::getline(std::cin, str);
		//std::cout << str;
		if (!wrong(str))
		{
			std::cout << "wrong message\n";
			goto A;
		}
		for (int i = 0; i < 4; ++i)
			msg[i] = str[i];
		if (send(s, msg, 4, 0) < 0)
		{
			printf("Запрос не удался.\n");
			return -1;
		}
	}

	if (ret == -1 || gotit)
	{
		printf("error %d\n", ret);
	}
	closesocket(s);
	WSACleanup();
	return 0;
}

