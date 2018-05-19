/*
 * MQTTClient.h
 *
 *  Created on: 07.04.2018
 *      Author: jochen
 */

#ifndef MQTTCLIENT_H_
#define MQTTCLIENT_H_

#include <mosquittopp.h>
#include <string>

class MQTTClient : protected mosqpp::mosquittopp {
public:
	MQTTClient(std::string topic);
	virtual ~MQTTClient();

	int stop();
	int start();

	int sendStatus(int channel, std::string subTopic, std::string status);

private:
	void on_message(const struct mosquitto_message *);
	std::string topic;
};

#endif /* MQTTCLIENT_H_ */
