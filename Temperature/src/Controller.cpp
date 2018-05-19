/*
 * Controller.cpp
 *
 *  Created on: 02.04.2018
 *      Author: jochen
 */

#include "Controller.h"
#include "Sensors.h"
#include "Utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>  // mkfifo
#include <sys/stat.h>   // mkfifo

#define PATH_NAME "/var/run/sensor/"
#define FIFO_NAME "fifo"

Controller& Controller::instance() {
	static Controller _instance;
	return _instance;
}

Controller::Controller() : running(false) {

}

Controller::~Controller() {
	// TODO Auto-generated destructor stub
}

int Controller::serverLoop() {
	// create FIFO Pipe

	umask(0);
	Utils::mkpath(PATH_NAME, 0777);

	unlink(PATH_NAME FIFO_NAME);
	int ret = mkfifo(PATH_NAME FIFO_NAME, 0777);

//	std::cout << "pipe create " << ret << std::endl;

	running = true;

	Buffer in;

	while (running) {
//		std::cout << "open pipe" << std::endl;

		int fd = ::open(PATH_NAME FIFO_NAME, O_RDONLY);

//		std::cout << "open pipe: " << fd << " " << errno << std::endl;

		if (in.receive(fd)) {

			LocalCommand* cmd = LocalCommand::read(in);

			if (cmd != NULL) {

//				std::cout << "Received command" << std::endl;

				cmd->execute();
				delete cmd;
			}
		}
	}
	return 0;
}

void Controller::stop() {
	running = false;
}

int Controller::execute(RemoteCommand& cmd) {

	Buffer out;

	cmd.write (out);

	int fd = ::open(PATH_NAME FIFO_NAME, O_WRONLY | O_NONBLOCK);

//	std::cout << "open pipe: " << fd << " " << errno << std::endl;

	out.send(fd);

	::close(fd);

	std::cout << "done" << std::endl;

	return 0;
}
