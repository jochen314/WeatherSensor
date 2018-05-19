//============================================================================
// Name        : Temperature.cpp
// Author      : Jochen Klein
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <string.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <unordered_map>
#include <stdlib.h>
#include <math.h>

#include "Command.h"
#include "Sensors.h"
#include "Lexer.h"

struct MatchPathSeparator {
	bool operator()(char ch) const {
		return ch == '/';
	}
};

std::string basename(std::string const& pathname) {
	return std::string(
			std::find_if(pathname.rbegin(), pathname.rend(),
					MatchPathSeparator()).base(), pathname.end());
}

void usage(std::string const& basename) {
	std::cerr << "usage: " << basename << " [command] " << std::endl
			<< "commands:" << std::endl
			<< "\tdaemon" << std::endl
			<< "\t\tstart\t\tstarts the daemon" << std::endl
			<< "\t\tstop\t\tstops the daemon" << std::endl
			<< "\tchannel [1|2|3]\tchanges the values of a channel" << std::endl
			<< "\t\tid <id>\t\tsets the id" << std::endl
			<< "\t\talarm [on|off]\t\tturns the alarm on or off" << std::endl
			<< "\t\tbattery [ok|bad]\t\tsets or resets the 'batterOK' flag" << std::endl
			<< "\t\ttermperature <value>\t\tsets the temperature in °C [-200..200] 0.1 degree precision" << std::endl
			<< "\t\thumidity <value>\t\tsets the humidity in %" << std::endl
			;
	exit(1);
}

DaemonCommand * parseDaemonCommand(std::list<std::string> args) {

	if (args.size() == 0) {
		return new LocalDaemonStartCommand();
	}
	if (args.size() != 1) {
		throw std::invalid_argument("too many arguments");
	}
	std::string cmd = args.front();

	Token word = Lexer::instance().toToken(cmd);

	switch (word.type()) {
	case TokenType::START:
		return new LocalDaemonStartCommand();
	case TokenType::STOP:
		return new RemoteDaemonStopCommand();
	case TokenType::RELOAD:
		return new RemoteDaemonReloadCommand();
	default:
		throw std::invalid_argument("unkown daemon command");
	}
}

UpdateCommand * parseUpdateCommand(std::list<std::string> args) {
	if (args.size() == 0) {
		throw std::invalid_argument("missing channel number");
	}
	std::string arg = args.front();
	args.pop_front();

	Token channel = Lexer::instance().toToken(arg);
	UpdateCommand* command = new RemoteUpdateCommand((int)channel - 1);

	while (args.size() > 0) {
		std::string cmd = args.front();
		args.pop_front();

		if (args.size() == 0) {
			throw std::invalid_argument("property argument missing");
		}

		arg = args.front();
		args.pop_front();

		Token word = Lexer::instance().toToken(cmd);
		Token para = Lexer::instance().toToken(arg);

		switch (word.type()) {
		case TokenType::ID: {
			command->id((int)para);
		}
			break;
		case TokenType::ALARM: {
			switch (para.type()) {
			case TokenType::ON:
				command->alarm(true);
				break;

			case TokenType::OFF:
				command->alarm(false);
				break;
			default:
				throw std::invalid_argument("unkown alarm state");
			}
		}
			break;
		case TokenType::BATTERY: {
			switch (para.type()) {
			case TokenType::BAD:
				command->batteryOK(false);
				break;

			case TokenType::OK:
				command->batteryOK(true);
				break;
			default:
				throw std::invalid_argument("unkown battery state");
			}
		}
			break;
		case TokenType::TEMPERATURE: {
			command->temperature((int)(10 * para));
		}
			break;
		case TokenType::HUMIDITY: {
			command->humidity((int)para);
		}
			break;
		default:
			throw std::invalid_argument("unkown sensor property");
		}
	}

	return command;
}

Command * parseCommand(std::list<std::string> args) {

	if (args.size() == 0) {
		throw std::invalid_argument("no command found");
	}

	std::string cmd = args.front();
	args.pop_front();

	Token word = Lexer::instance().toToken(cmd);

	switch (word.type()) {
	case TokenType::DAEMON:
		return parseDaemonCommand(args);
	case TokenType::CHANNEL:
		return parseUpdateCommand(args);
	default:
		throw std::invalid_argument("unkown command");
	}
}

int main(int argc, char ** argv) {
	std::string executable = basename(argv[0]);

	std::list<std::string> arguments;
	for (int i = 1; i < argc; i++) {
		arguments.push_back(argv[i]);
	}

	try {
		Command * command = parseCommand(arguments);
		return command->execute();

	} catch (std::invalid_argument& e) {
		usage(executable);
	}
}
