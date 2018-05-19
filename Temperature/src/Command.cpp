/*
 * RemoteCommand.cpp
 *
 *  Created on: 30.03.2018
 *      Author: jochen
 */

#include "Command.h"
#include "Controller.h"
#include "Sensors.h"
#include <string.h>

void FieldSet::write(Buffer& out) {
	unsigned long int bits = _is_set.to_ulong();
	out.write(&bits, 1);
}

void FieldSet::read(Buffer& in) {
	unsigned long int bits(0);
	in.read(&bits, 1);
	_is_set = std::bitset<6>(bits);
}

RemoteCommand::~RemoteCommand() {
}

RemoteUpdateCommand::RemoteUpdateCommand(uint8_t channel) {
	_channel = channel;
}

int RemoteUpdateCommand::execute() {
	return Controller::instance().execute(*this);
}

enum CommandType {
	UPDATE, DAEMON_STOP, DAEMON_RELOAD
};

void RemoteUpdateCommand::write(Buffer& out) {

	enum CommandType type = UPDATE;

	out.write(&type, sizeof(type));
	out.write(&_channel, sizeof(_channel));
	_fields.write(out);
	if (_fields.test(FieldSet::Field::ID)) {
		out.write(&_id, sizeof(_id));
	}
	if (_fields.test(FieldSet::Field::ALARM)) {
		out.write(&_alarm, sizeof(_alarm));
	}
	if (_fields.test(FieldSet::Field::BATTERY)) {
		out.write(&_battery, sizeof(_battery));
	}
	if (_fields.test(FieldSet::Field::TEMPERATUE)) {
		out.write(&_temperature, sizeof(_temperature));
	}
	if (_fields.test(FieldSet::Field::HUMIDITY)) {
		out.write(&_humidity, sizeof(_humidity));
	}
}

RemoteDaemonStopCommand::RemoteDaemonStopCommand() {
}

void RemoteDaemonStopCommand::write(Buffer& out) {
	enum CommandType type = DAEMON_STOP;

	out.write(&type, sizeof(type));
}

int RemoteDaemonStopCommand::execute() {
	return Controller::instance().execute(*this);
}

RemoteDaemonReloadCommand::RemoteDaemonReloadCommand() {
}

void RemoteDaemonReloadCommand::write(Buffer& out) {
	enum CommandType type = DAEMON_RELOAD;

	out.write(&type, sizeof(type));
}

int RemoteDaemonReloadCommand::execute() {
	return Controller::instance().execute(*this);
}

LocalCommand::~LocalCommand() {
}

LocalDaemonStartCommand::LocalDaemonStartCommand() {
}

int LocalDaemonStartCommand::execute() {
	Sensors::instance().startDaemon();
	return 0;
}

LocalDaemonStopCommand::LocalDaemonStopCommand() {
}

int LocalDaemonStopCommand::execute() {
	Controller::instance().stop();
	return 0;
}

LocalDaemonReloadCommand::LocalDaemonReloadCommand() {
}

int LocalDaemonReloadCommand::execute() {
	Sensors::instance().load();
	return 0;
}

LocalUpdateCommand::LocalUpdateCommand() {
}

LocalUpdateCommand::LocalUpdateCommand(uint8_t channel) {
	_channel = channel;
}

int LocalUpdateCommand::execute() {
	Sensors::instance().update(*this);
	return 0;
}

void LocalUpdateCommand::read(Buffer& in) {
	in.read(&_channel, sizeof(_channel));
	_fields.read(in);

	if (_fields.test(FieldSet::Field::ID)) {
		in.read(&_id, sizeof(_id));
	}
	if (_fields.test(FieldSet::Field::ALARM)) {
		in.read(&_alarm, sizeof(_alarm));
	}
	if (_fields.test(FieldSet::Field::BATTERY)) {
		in.read(&_battery, sizeof(_battery));
	}
	if (_fields.test(FieldSet::Field::TEMPERATUE)) {
		in.read(&_temperature, sizeof(_temperature));
	}
	if (_fields.test(FieldSet::Field::HUMIDITY)) {
		in.read(&_humidity, sizeof(_humidity));
	}
}

LocalCommand *LocalCommand::read(Buffer& in) {
	enum CommandType type;

	in.read((char*) &type, sizeof(type));

	switch (type) {
	case CommandType::DAEMON_STOP: {
		return new LocalDaemonStopCommand();
	}
	case CommandType::DAEMON_RELOAD: {
		return new LocalDaemonReloadCommand();
	}
	case CommandType::UPDATE: {
		LocalUpdateCommand* cmd = new LocalUpdateCommand();
		cmd->read(in);
		return cmd;
	}
	default:
		throw std::istream::failure("Unkown Command");
	}
}
