#include "OFProtocol.h"

int OFProtocol::copyNonNullBytes(byte *dest, byte *source, int maxSize) {
	byte *nonNullBytes = (byte *) calloc(1, maxSize);
	int nonNullLength;
	for (nonNullLength = 0; source[nonNullLength] != 00 && nonNullLength <= maxSize; nonNullLength++) {
		nonNullBytes[nonNullLength] = source[nonNullLength];
	}

	memset(dest, 0, maxSize);
	memcpy(dest, nonNullBytes, nonNullLength);
	free(nonNullBytes);
	return nonNullLength;
}

OFProtocol::OF_PACKET_TYPE OFProtocol::getType(byte *traffic) {
	// check completeness
	if (traffic[0] != 0xBB && traffic[299] != 0xEE) {
		return OFProtocol::OF_PACKET_TYPE::INCOMPLETE;
	}

	// check for type
	if (traffic[1] == 0x1C) {
		return OFProtocol::OF_PACKET_TYPE::MESSAGE;
	}

	return OFProtocol::OF_PACKET_TYPE::INVALID;
}
