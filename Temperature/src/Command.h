/*
 * SensorUpdater.h
 *
 *  Created on: 28.03.2018
 *      Author: jochen
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <bitset>
#include <cstdint>
#include <iostream>

#include "Buffer.h"

class FieldSet {
public:
	enum Field {
		ID, CHANNEL, ALARM, BATTERY, TEMPERATUE, HUMIDITY
	};

	FieldSet() :
			_is_set() {
	}

	FieldSet& operator|=(Field f) {
		_is_set.set(static_cast<int>(f));
		return *this;
	}

	bool test(Field f) const {
		return _is_set.test(static_cast<int>(f));
	}

	void read(Buffer& in);
	void write(Buffer& out);

private:
	std::bitset<6> _is_set;
};

class Command {
public:
	virtual ~Command() {
	}

	virtual int execute() = 0;
};

class LocalCommand;

class UpdateCommand: public Command {
	friend std::ostream& operator<<(std::ostream& out, const UpdateCommand& cmd);
	friend LocalCommand * readLocalUpdateCommand(std::istream& in);
	friend class Sensors;

protected:
	FieldSet _fields;
	uint8_t _channel;
	uint8_t _id;
	bool _alarm;
	bool _battery;
	int _temperature;
	int _humidity;
public:
	void id(uint8_t id) {
		_fields |= FieldSet::Field::ID;
		_id = id;
	}
	void alarm(bool flag) {
		_fields |= FieldSet::Field::ALARM;
		_alarm = flag;
	}
	void batteryOK(bool flag) {
		_fields |= FieldSet::Field::BATTERY;
		_battery = flag;
	}
	void temperature(int temperature) {
		_fields |= FieldSet::Field::TEMPERATUE;
		_temperature = temperature;
	}
	void humidity(int percent) {
		_fields |= FieldSet::Field::HUMIDITY;
		_humidity = percent;
	}
};

class DaemonCommand: public Command {
};

class DaemonStopCommand: public DaemonCommand {
};

class DaemonStartCommand: public DaemonCommand {
};

class DaemonReloadCommand: public DaemonCommand {
};

//
//
// Remote Commands
//
//

class RemoteCommand: public Command {
public:
	virtual ~RemoteCommand();

	virtual void write(Buffer& out) = 0;
};

class RemoteDaemonStopCommand: public RemoteCommand, public DaemonStopCommand {
public:
	RemoteDaemonStopCommand();

	int execute();

	void write(Buffer& out);
};

class RemoteDaemonReloadCommand: public RemoteCommand, public DaemonReloadCommand {
public:
	RemoteDaemonReloadCommand();

	int execute();

	void write(Buffer& out);
};

class RemoteUpdateCommand: public RemoteCommand, public UpdateCommand {
public:
	RemoteUpdateCommand(uint8_t channel);

	int execute();
	void write(Buffer& out);
};


//
//
// Daemon side
//
//

class LocalCommand: public Command {
public:
	virtual ~LocalCommand();

	static LocalCommand* read(Buffer& in);
};

class LocalDaemonStartCommand: public LocalCommand, public DaemonStartCommand {
public:
	LocalDaemonStartCommand();
	int execute();
	void read(Buffer& in);
};

class LocalDaemonReloadCommand: public LocalCommand, public DaemonReloadCommand {
public:
	LocalDaemonReloadCommand();
	int execute();
	void read(Buffer& in);
};

class LocalDaemonStopCommand: public LocalCommand, public DaemonStopCommand {
public:
	LocalDaemonStopCommand();
	int execute();
	void read(Buffer& in);
};

class LocalUpdateCommand: public LocalCommand, public UpdateCommand {
public:
	LocalUpdateCommand();
	LocalUpdateCommand(uint8_t channel);
	int execute();
	void read(Buffer& in);
};

#endif /* COMMAND_H_ */
