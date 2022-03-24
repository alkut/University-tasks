#include <iostream>
#include <thread>
#include <string.h>
#include <vector>
#include <time.h>


#pragma comment(lib,"Ws2_32.lib")
#include <Winsock2.h>

#define DEFAULT_BUFLEN 512

int cow_cnt(const char* s, const char* ans)
{
	int a = 0;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			if (s[i]==ans[j]) ++a;
	return a;
}

int bull_cnt(const char* s, const char* ans)
{
	int a = 0;
	for (int i = 0; i < 4; ++i)
		if (s[i] == ans[i]) ++a;
	return a;
}

void gen(char* ans)
{
	std::string v;
	for (char i = '1'; i <= '9'; ++i)
		v.push_back(i);
	for (int i = 0; i < 4; ++i)
	{
		char tmp = v[rand() % (9 - i)];
		ans[i] = tmp;
		std::remove(v.begin(), v.end(), tmp);
	}
}

bool stop(const char* s)
{
	return s[0] == 's' && s[1] == 't' && s[2] == 'o' && s[3] == 'p';
}

void describe_err()
{
	switch (WSAGetLastError())
	{
	case WSANOTINITIALISED:
	{
		std::cout << "init winsock\n"; break;
	}
	case WSAENETDOWN:
	{
		std::cout << "inet fail\n"; break;
	}
	default:
	{
		std::cout << "strange error\n"; break;
	}
	}
}


void add_client(SOCKET& s_srv)
{
	sockaddr_in client;
	int sof = sizeof(sockaddr_in);
	SOCKET s_client = accept(s_srv, reinterpret_cast<sockaddr*>(&client), &sof);
	if (s_client == INVALID_SOCKET)
	{
		describe_err();
		return;
	}
	char recvbuf[DEFAULT_BUFLEN+1];
	char sbuf[DEFAULT_BUFLEN] = "print stop to stop the server\n";
	char def[DEFAULT_BUFLEN] = "you have 0 cows, 0 bulls\n";
	char win[DEFAULT_BUFLEN] = "you win !!! \nstarting next round \n";
	char ans[4];
	gen(ans);
	int cow, bull;
	int recvbuflen = DEFAULT_BUFLEN;
	int sbuflen = DEFAULT_BUFLEN;
	int byte_recieved;
	send(s_client, sbuf,35, 0);
	do
	{
		byte_recieved = recv(s_client, recvbuf, 4, 0);
		if (byte_recieved >= 4 && stop(recvbuf))
		{
			send(s_client, "server stopped \n", 16, 0);
			return;
		}
		if (byte_recieved >= 4) {
			cow = cow_cnt(recvbuf, ans);
			bull = bull_cnt(recvbuf, ans);
			if (bull == 4)
			{
				send(s_client, win, 35, 0);
				gen(ans);
				continue;
			}
			def[9] += cow;
			def[17] += bull;
			send(s_client, def, 24, 0);
			def[9] -= cow;
			def[17] -= bull;
		}

	} while (byte_recieved > 0);
	closesocket(s_client);
}

int main()
{
	srand(time(NULL));
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}



	SOCKET s_srv = socket(AF_INET, SOCK_STREAM, 0);
	if (s_srv == INVALID_SOCKET)
	{
		std::cout << "can't create socket\n";
		return -1;
	}



	/*
	typedef struct sockaddr_in {
  short          sin_family;
  u_short        sin_port;
  struct in_addr sin_addr;
  char           sin_zero[8];
   } SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;

   where

   struct in_addr {
  union {
	struct {
	  u_char s_b1;
	  u_char s_b2;
	  u_char s_b3;
	  u_char s_b4;
	} S_un_b;
	struct {
	  u_short s_w1;
	  u_short s_w2;
	} S_un_w;
	u_long S_addr;
  } S_un;
};
	*/

	sockaddr_in srv;
	memset(&srv, 0, sizeof(srv));

	srv.sin_family = AF_INET;
	//srv.sin_addr.s_addr = INADDR_ANY; 
	srv.sin_port = htons(5555);
	int err = bind(s_srv, reinterpret_cast<sockaddr*>(&srv), sizeof(srv));
	if (err != 0)
	{
		describe_err();
		return -2;
	}
	err = listen(s_srv, 2);
	if (err != 0)
	{
		describe_err();
		return -3;
	}
	add_client(s_srv);
	closesocket(s_srv);
	WSACleanup();
	return 0;
}
