#pragma once
#include "InPacketListener.h"
#include <WinSock2.h>
#include <Windows.h>


typedef struct {
	SOCKET Socket;
	SOCKADDR_STORAGE CliendAddr;
} PER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF databuf;
	CHAR buffer[8192];
	DWORD bytesSend;
	DWORD bytesRecv;
} PER_IO_OPERATION_DATA;

typedef struct {
	SOCKET *ofConnection;
	InPacketListener *inPacketListener;
	int localPort;
	bool *status;
} WorkerArgs;

/* Duplex communication with Openfire plugin */
class OFSerConnection {
private:
	// connection parameters
	const int LOCAL_PORT;

	// listeners
	InPacketListener *inPacketListener;

	// Winsock
	SOCKET **OFConnection;
	bool *status;
	SOCKADDR_IN serverAddr;
	//WorkerArgs *workerArgs;

	DWORD workerThreadID;

public:
	OFSerConnection(int localPort);

	// set incoming packet listener
	void setInPacketListener(InPacketListener *listener);

	// Establish two socket connections:
	// To Openfire Server and From Openfire Server
	// returns a pointer to the real time status of the connection
	// if the connection is normal, the returned pointer will contain
	// true, otherwise it will be false.
	// NOTICE: this pointer is updated in realtime. 
	bool* start();

	// returns boolean pointer containing bool that indicates the connections
	// current status
	bool* getStatus();


	// Send message through established OFConnection.
	// Returns the bytes sent.
	// Return -1 if connection is at error
	int sendPacket(const char *packet, int length);

	// close both socket connections
	// returns true if successfully closed
	bool close();
};