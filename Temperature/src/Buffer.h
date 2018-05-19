/*
 * Buffer.h
 *
 *  Created on: 03.04.2018
 *      Author: jochen
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <iostream>

class Buffer {
public:
	Buffer();
	~Buffer();

	void read(void *, int);
	void write(void *, int);

	void send(int fd);
	bool receive(int fd);
private:
	friend std::ostream& operator<< (std::ostream& out, Buffer& buffer);
	friend std::istream& operator>> (std::istream& in, Buffer& buffer);

	void * buffer;
	int size;
	int pos;
};

#endif /* BUFFER_H_ */
