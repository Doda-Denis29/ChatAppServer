#include "myLibs.h"
#include "defines.h"

#pragma comment (lib, "ws2_32.lib")

void setUpBind(sockaddr_in& someStruct, int PORT)
{
	someStruct.sin_family = AF_INET;
	someStruct.sin_port = htons(PORT);
	someStruct.sin_addr.S_un.S_addr = INADDR_ANY;
}

void runApp(SOCKET s, fd_set m)
{
	bool run = true;
	const char* wMessage = "Connected to the server";
	std::string commandControl{};
	std::ostringstream oss{};
	while (run)
	{
		fd_set cpy = m;
		int sCount = select(0, &cpy, nullptr, nullptr, nullptr);
		for (auto in = 0; in < sCount; in++)
		{
			SOCKET newS = cpy.fd_array[in];
			if (newS == s)
			{
				SOCKET client = accept(s, nullptr, nullptr);
				FD_SET(client, &m);
				send(newS, wMessage, 24, 0);
			}
			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);
				int bytes = recv(newS, buf, 4096, 0);
				if (bytes <= 0)
				{
					closesocket(newS);
					FD_CLR(newS, &m);
				}
				else
				{
					if (buf[0] == '\\')
					{
						commandControl = std::string(buf, bytes);
						if (commandControl == "\\quit")
						{
							run = false;
							break;
						}
						continue; // Maybe add more commands in the future
					}
					for (std::size_t j = 0; j < m.fd_count; j++)
					{
						SOCKET newOutS = m.fd_array[j];
						if (newOutS == s)
						{
							continue;
						}
						if (newOutS != newS)
						{
							oss << "USER_" << newS << ": " << buf << "\r\n";
						}
					}
				}
			}
		}
	}
}

int main()
{
	WSADATA wsD;
	WORD word = MAKEWORD(2, 2); // Init the winsock

	int wsCheck = WSAStartup(word, &wsD);
	if (wsCheck != 0)
	{
		std::cerr << "Can't init the winsock! Exitting ... \n";
		return -1;
	}

	sockaddr_in bint;
	setUpBind(bint, PORT);
	
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket since it is invalid \n";
		return -1;
	}

	bind(listening, (sockaddr*)&bint, sizeof(bint));

	listen(listening, SOMAXCONN); // To tell the winsock what socket is for listening

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);
}