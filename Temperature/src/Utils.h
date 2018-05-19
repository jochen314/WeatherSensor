/*
 * Utils.h
 *
 *  Created on: 06.05.2018
 *      Author: jochen
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
#include <sys/stat.h>

class Utils {
private:
	Utils();
	virtual ~Utils();

public:
	static int mkpath(std::string s, mode_t mode);
};

#endif /* UTILS_H_ */
