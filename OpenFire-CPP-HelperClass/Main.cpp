#include <iostream>
#include "OFSerConnection.h"
#include <chrono>
#include <thread>
#include <sstream>
#include "OFProtocol.h"
#include "OFMessage.h"


using namespace std;

// custom listener class
class InTrafficListener: public InPacketListener {
public:
	void OnPacketReceived(char *packetBuffer) {
		std::cout << "OFSerConn: Received --> ";
		printf("%s", (char *) packetBuffer);
		std::cout << std::endl;

 	}
};

int main() {
	OFSerConnection *ofSerConnection
		= new OFSerConnection(533);
	InTrafficListener *listener = new InTrafficListener();
	ofSerConnection->setInPacketListener(listener);
	ofSerConnection->start();

	int i = 0;
	
	cin.get();
}