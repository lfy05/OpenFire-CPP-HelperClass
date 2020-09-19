#pragma once
#include <stdlib.h>
#include <string>
#include "OFProtocol.h"
#include "OFPackets.h"

class OFMessage : public OFPackets {
	friend class OFProtocol;
private:
	char *from;
	int senderIDLength;

	char *to;
	int receiverIDLength;

	char *body;
	int bodyLength;

	const byte header = 0xBB;
	const byte flag = 0X1C;
	const byte toMobilePlatform = 0x2E;
	const byte footer = 0xEE;

public:
	OFMessage();
	char *getFrom();
	char *getTo();
	char *getBody();

	void setFrom(std::string from);
	void setTo(std::string to);
	void setBody(std::string body);

	void setFrom(byte *from, int length);
	void setTo(byte *to, int length);
	void setBody(byte *body, int length);

	// returned pointer is allocated with calloc. must free when no longer use.
	byte* getOFBytes();

	static OFMessage* getFromTraffic(byte *traffic);

};