/*
 * Buffer.cpp
 *
 *  Created on: 03.04.2018
 *      Author: jochen
 */

#include "Buffer.h"
#include <string.h>
#include <unistd.h>

Buffer::Buffer() :
		buffer(NULL), size(0), pos(0) {
}

Buffer::~Buffer() {
	if (buffer != NULL) {
		free(buffer);
	}
}

void Buffer::write(void *data, int size) {
	int newSize = this->size;
	while (pos + size >= newSize) {
		newSize = newSize == 0 ? 16 : 2 * newSize;
	}
	if (newSize != this->size) {
		void* newBuffer = malloc(newSize);
		memset(newBuffer, 0, newSize);
		if (buffer != NULL) {
			if (pos > 0) {
				memcpy(newBuffer, buffer, pos);
			}
			free(buffer);
		}
		buffer = newBuffer;
		this->size = newSize;
	}
//	std::cout << "write (" << std::dec << size << ") " << std::hex;
//	for (int i = 0; i < size; i++) {
//		std::cout << (int) *(((unsigned char *) data) + i) << " ";
//	}
//	std::cout << std::endl;
	memcpy(((char*) buffer) + pos, data, size);
	pos += size;
}

void Buffer::read(void *data, int size) {

	if (this->pos + size <= this->size) {
//		std::cout << "read (" << std::dec << size << ") " << std::hex;
//		for (int i = 0; i < size; i++) {
//			std::cout << (int) *(((unsigned char *) buffer) + pos + i) << " ";
//		}
//		std::cout << std::endl;

		memcpy(data, ((char *) buffer) + pos, size);
		pos += size;
	}
}

void Buffer::send(int fd) {
//	std::cout << "send (" << std::dec << pos << ") " << std::hex;
//	for (int i = 0; i < pos; i++) {
//		std::cout << (int) *(((unsigned char *) buffer) + i) << " ";
//	}
//	std::cout << std::endl;

	::write(fd, &pos, sizeof(pos));
	if (pos > 0) {
		::write(fd, buffer, pos);
	}
}

bool Buffer::receive(int fd) {
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
	size = 0;
	pos = 0;
	if (::read(fd, &size, sizeof(size)) != sizeof(size)) {
		return false;
	}
//	std::cout << "received(" << std::dec << size << ") " << std::hex;

	if (size != 0) {
		buffer = malloc(size);
		if (::read(fd, buffer, size) != size) {
			return false;
		}
	}

//	for (int i = 0; i < size; i++) {
//		std::cout << (int) *(((unsigned char *) buffer) + i) << " ";
//	}
//	std::cout << std::endl;

	return true;
}
