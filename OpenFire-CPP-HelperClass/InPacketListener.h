#pragma once

/* Listener class that helps handling */
class InPacketListener {
public:
	// virtual callback method
	// define the behavior after receiving a packet
	// Override for custom behavior
	virtual void OnPacketReceived(char *packetBuffer);
};
