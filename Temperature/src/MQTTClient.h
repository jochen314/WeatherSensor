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
#include <json.hpp>
using json = nlohmann::json;

class MQTTClient : protected mosqpp::mosquittopp {
public:
	MQTTClient();
	virtual ~MQTTClient();

	int stop();
	int start();

	int sendStatus(int channel, std::string subTopic, std::string status);

private:
	void on_message(const struct mosquitto_message *);
	std::string _id;
	std::string _topic;
	std::string _host;
	int _port;

	friend void to_json(json& j, const MQTTClient& c) ;
	friend void from_json(const json& j, MQTTClient& c) ;
};

void to_json(json& j, const MQTTClient& c) ;
void from_json(const json& j, MQTTClient& c) ;


#endif /* MQTTCLIENT_H_ */
