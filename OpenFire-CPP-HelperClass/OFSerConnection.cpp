#include "OFSerConnection.h"
// Winsock  
#include <iostream>
#include <Windows.h>
#include "InPacketListener.h"

#pragma comment(lib, "Ws2_32.lib")

OFSerConnection::OFSerConnection(int localPort)
	:LOCAL_PORT(localPort){};

// Worker Thread Routine
// NOTICE: not to be used independently
DWORD WINAPI WorkerThreadRoutine(LPVOID workerArgs) {
REVIVE:

	// retrieve arguments
	WorkerArgs *args = (WorkerArgs *) workerArgs;
	args->ofConnection = (SOCKET *) calloc(1, sizeof(SOCKET));

	// create new listening socket
	SOCKET listeningSocket;
	listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listeningSocket == INVALID_SOCKET) {
		std::cout << "OFSerConn: listening socket creation FAILED: " << WSAGetLastError() << " Terminated." << std::endl;
		*(args->status) = false;
		goto REVIVE;
	} else {
		std::cout << "OFSerConn: listening socket creation OKAY." << std::endl;
	}

	// setup SOCKADDR_IN structure
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(args->localPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket with SOCKADDR_IN
	if (bind(listeningSocket, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "OFSerConn: binding listening socket failed: " << WSAGetLastError() << " Terminated.";
		closesocket(listeningSocket);
		*(args->status) = false;
		std::cout << "OFSerConn: Attempting revive...." << std::endl;
		goto REVIVE;
	} else {
		std::cout << "OFSerConn: bind listening socket OKAY, port " << ntohs(serverAddr.sin_port) << std::endl;
	}

	// start listening
	if (listen(listeningSocket, 1) == SOCKET_ERROR) {
		std::cout << "OFSerConn: start listening FAILED: " << WSAGetLastError() << std::endl;
		closesocket(listeningSocket);
		*(args->status) = false;
		std::cout << "OFSerConn: Attempting revive...." << std::endl;
		goto REVIVE;
	} else {
		std::cout << "OFSerConn: start listening OKAY" << std::endl;
	}

	// accept connection
	*(args->ofConnection) = accept(listeningSocket, NULL, NULL);
	if (*(args->ofConnection) == SOCKET_ERROR) {
		std::cout << "OFSerConn: Connection with OFServer FAILED" << std::endl;
		closesocket(listeningSocket);
		*(args->status) = false;
		std::cout << "OFSerConn: Attempting revive...." << std::endl;
		goto REVIVE;
	} else {
		*(args->status) = true;
		std::cout << "OFSerConn: Connection with OFServer ESTABLISHED" << std::endl;
		closesocket(listeningSocket);
		std::cout << "OFSerConn: Listening socket closed" << std::endl;
	}

	// set the socket to be non-blocking
	unsigned long isNonBlock = 1;
	if (ioctlsocket(*(args->ofConnection), FIONBIO, &isNonBlock) == SOCKET_ERROR) {
		std::cout << "OFSerConn: Set accept socket non-blocking FAILED: " << WSAGetLastError() << std::endl;
		closesocket(listeningSocket);
		*(args->status) = false;
		std::cout << "OFSerConn: Attempting revive...." << std::endl;
		goto REVIVE;
	} else {
		std::cout << "OFSerConn: Set accept socket non-blocking OKAY. " << std::endl;
	}

	// prepare fd_set for select()
	fd_set readfds;

	// prepare buffer
	char *recvBuffer = new char[1000];
	memset(recvBuffer, 0, sizeof(recvBuffer));

	// start loop
	while (true) {
		// clear readfds
		FD_ZERO(&readfds);

		// add accepted socket into readfds
		FD_SET(*(args->ofConnection), &readfds);

		// wait for incoming packet
		int availSocket = select(0, &readfds, NULL, NULL, NULL);
		if (availSocket > 0 && FD_ISSET(*(args->ofConnection), &readfds)) {
			// empty buffer
			memset(recvBuffer, 0, sizeof(recvBuffer));

			// retrieve packet
			int len = recv(*(args->ofConnection), recvBuffer, 10240, 0);
			if (len > 0) {
				// append null to the end of buffer to terminate
				recvBuffer[len] = NULL;

				// notify listener
				args->inPacketListener->OnPacketReceived(recvBuffer);

			} else if (len == -1) {
				// if client drops connection, close socket and exit loop
				closesocket(*(args->ofConnection));
				std::cout << "OFSerConn: OF disconnected, current socket closed. " << std::endl;
				*(args->status) = false;
				std::cout << "OFSerConn: Attempting revive...." << std::endl;
				goto REVIVE;

			} else {
				// other error occurs
				std::cout << "OFSerConn: Error: " << WSAGetLastError() << " Terminated." << std::endl;
				closesocket(*(args->ofConnection));
				*(args->status) = false;
				std::cout << "OFSerConn: Attempting revive...." << std::endl;
				goto REVIVE;
			}
		}
	}
		
	return 0;
}

void OFSerConnection::setInPacketListener(InPacketListener *listener) {
	this->inPacketListener = listener;
}

bool* OFSerConnection::start() {
	// initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "OFSerConn: WSAStartup Failed" << WSAGetLastError() << std::endl;
		WSACleanup();
		return (bool *)calloc(1, sizeof(bool));
	}

	// create worker thread that blocks on listening to the port
	WorkerArgs *workerArgs = (WorkerArgs*) calloc(1, sizeof(workerArgs));		// don't free workerArgs. It is used in the workerthread.
	//workerArgs->ofConnection = OFConnection;
	workerArgs->inPacketListener = inPacketListener;
	workerArgs->localPort = LOCAL_PORT;
	workerArgs->status = (bool *) calloc(1, sizeof(bool));
	this->status = (bool *) calloc(1, sizeof(bool));
	this->status = workerArgs->status;
	HANDLE workerThreadHandle = CreateThread(0, 0, WorkerThreadRoutine, workerArgs, 0, &workerThreadID);

	// wait until Socket is ready
	while (!*(workerArgs->status));

	// preserve OF socket
	OFConnection = &(workerArgs->ofConnection);
	return workerArgs->status;
}

bool *OFSerConnection::getStatus() {
	return this->status;
}

int OFSerConnection::sendPacket(const char *packet, int length) {
	// check if the connection is ready
	if (!*(this->getStatus())) {
		std::cout << "OFSerConn: Connection not available. Sending " << packet << " FAILED.";
		return -1;
	} else {
		return send(**OFConnection, packet, length, 0);
	}
}