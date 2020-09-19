#include "OFMessage.h"

OFMessage::OFMessage() {
	this->from = (char *) calloc(1, 64);
	this->to = (char *) calloc(1, 64);
	this->body = (char *) calloc(1, 167);
}

char* OFMessage::getFrom() {
	return (char *) from;
}

char* OFMessage::getTo() {
	return (char *) to;
}

char* OFMessage::getBody() {
	return (char *) body;
}

void OFMessage::setFrom(std::string from) {
	//this->from = from.c_str();
	memcpy(this->from, from.c_str(), from.length());
	this->senderIDLength = from.length();
}

void OFMessage::setFrom(byte *from, int length) {
	memcpy(this->from, from, length);
	senderIDLength = length;
}

void OFMessage::setTo(std::string to) {
	memcpy(this->to, to.c_str(), to.length());
	this->receiverIDLength = to.length();
}

void OFMessage::setTo(byte *to, int length) {
	memcpy(this->to, to, length);
	this->receiverIDLength = length;
}

void OFMessage::setBody(std::string body) {
	memcpy(this->body, body.c_str(), body.length());
	this->bodyLength = body.length();
}

void OFMessage::setBody(byte *body, int length) {
	memcpy(this->body, body, length);
	this->bodyLength = length;
}

byte* OFMessage::getOFBytes() {
	byte *OFBytes = (byte *) calloc(300, sizeof(byte));

	byte *index = OFBytes;

	// set header
	*index = header;
	index++;

	// set flag
	*index = flag;
	index++;

	// set target platform
	*index = toMobilePlatform;
	index++;

	// set sender ID
	byte *senderID = (byte *) this->getFrom();
	memcpy(index, senderID, senderIDLength);
	index += 64;

	// set receiver ID
	byte *receiverID = (byte *) this->getTo();
	memcpy(index, receiverID, receiverIDLength);
	index += 64;

	// set message body
	byte *body = (byte *) this->getBody();
	memcpy(index, body, bodyLength);
	index += 167;

	// set footer
	*index = footer;

	return OFBytes;
}

OFMessage* OFMessage::getFromTraffic(byte *traffic) {
	// check traffic completeness
	if (traffic[0] != 0xBB && traffic[299] != 0xEE) {
		return NULL;
	}

	// if traffic is a message packet
	if (traffic[1] == 0x1C) {
		OFMessage *msgPacket = new OFMessage;

		// point to beginning of sender ID
		byte *index = traffic + 3;

		// retrieve sender ID
		msgPacket->senderIDLength = OFProtocol::copyNonNullBytes((byte *) msgPacket->from, index, 64);
		index += 64;
		 
		// retrieve receiver ID
		msgPacket->receiverIDLength = OFProtocol::copyNonNullBytes((byte *) msgPacket->to, index, 64);
		index += 64;

		// retrieve message body
		msgPacket->bodyLength = OFProtocol::copyNonNullBytes((byte *) msgPacket->body, index, 167);
		index += 167;

		return msgPacket;
	}
}