/*
 * Controller.h
 *
 *  Created on: 02.04.2018
 *      Author: jochen
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Command.h"

class Controller {
public:
	static Controller& instance();

	int execute(RemoteCommand& cmd);

	int serverLoop();
	void stop();
private:
	Controller();
	virtual ~Controller();

	bool running;
};

#endif /* CONTROLLER_H_ */
