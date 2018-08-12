/*
 * Sensors.h
 *
 *  Created on: 28.03.2018
 *      Author: jochen
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include "Command.h"
#include "Sensor.hpp"
#include "MQTTClient.h"

class Sensors {
public:
	static Sensors& instance();

	virtual ~Sensors();

	void load();
	void save();

	UpdateCommand* update(uint8_t channel);

	int startDaemon();
private:
	Sensors();
	Sensors(const Sensors&);

	friend class LocalUpdateCommand;
	int update(const UpdateCommand& cmd);

	int pin;
	Sensor* sensors[3];

	friend void signal_handler(int signum);
	void alarm();

	void setupAlarm();
	void disableAlarm();
	void enableAlarm();

	pthread_mutexattr_t mutex_attr;
	pthread_mutex_t mutex;

	MQTTClient* client;
};



#endif /* SENSORS_H_ */
