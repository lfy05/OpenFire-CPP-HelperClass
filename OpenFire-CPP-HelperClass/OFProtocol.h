#pragma once
#include <stdlib.h>
#include <string>

typedef unsigned char byte;

class OFProtocol {
public:
	static enum OF_PACKET_TYPE{MESSAGE, PRESENCE, INVALID, INCOMPLETE};
	static int copyNonNullBytes(byte *dest, byte *source, int maxSize);
	static OFProtocol::OF_PACKET_TYPE getType(byte *traffic);
};

