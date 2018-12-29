/*
 * MQTTClient.cpp
 *
 *  Created on: 07.04.2018
 *      Author: jochen
 */

#include "MQTTClient.h"
#include "Sensors.h"
#include <iostream>
#include <cmath>
#include <cstdlib>


using namespace std;
MQTTClient::MQTTClient(std::string topic) :
		mosqpp::mosquittopp("sensor"), topic(topic) {
	connect("localhost");
	subscribe(NULL, ("cmnd/" + topic + "/#").c_str(), 0);
}

MQTTClient::~MQTTClient() {
	// TODO Auto-generated destructor stub
}
int MQTTClient::sendStatus(int channel, std::string subTopic,
		std::string status) {
	return publish(NULL,
			("status/" + topic + "/" + std::to_string(channel) + "/" + subTopic).c_str(),
			status.length(), status.c_str(), 0, true);
}

int MQTTClient::start() {
	return mosqpp::mosquittopp::loop_start();
}
int MQTTClient::stop() {
	mosqpp::mosquittopp::disconnect();
	return mosqpp::mosquittopp::loop_stop();
}

void MQTTClient::on_message(const struct mosquitto_message * msg) {
	char** subtopics;
	int len;

	mosqpp::sub_topic_tokenise(msg->topic, &subtopics, &len);

	if (len > 3) {
		int channel = atoi(subtopics[2]);

		if (channel > 0 && channel <= 3) {

			UpdateCommand * cmd = Sensors::instance().update(channel-1);

			if (cmd != NULL) {
				std::string value = subtopics[3];

				if ("temperature" == value) {
					double t = atof((char*)msg->payload);
					int temperature = nearbyint(10 * t);

					std::cout << "update channel " << channel << " temperature "
							<< temperature << endl;

					cmd->temperature(temperature);
				} else if ("humidity" == value) {
					int humidity = atoi((char*)msg->payload);

					std::cout << "update channel " << channel << " humidity "
							<< humidity << endl;

					cmd->humidity(humidity);
				}

				cmd->execute();

				delete cmd;
			}
		}
	}

	mosqpp::sub_topic_tokens_free(&subtopics, len);
}
