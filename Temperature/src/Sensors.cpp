/*
 * Sensors.cpp
 *
 *  Created on: 28.03.2018
 *      Author: jochen
 */

#include "Sensors.h"
#include "Controller.h"
#include "MQTTClient.h"
#include "Utils.h"

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
using namespace std;

#define CONFIG_PATH "/usr/local/etc/sensors/"
#define CONFIG_FILE "config"

void signal_handler(int signum);
void set_max_priority();

Sensors& Sensors::instance() {
	static Sensors _instance;
	return _instance;
}

Sensors::Sensors() : pin (9) {
	memset(sensors, 0, sizeof(sensors));
	wiringPiSetup();

	pthread_mutexattr_init(&mutex_attr);
	pthread_mutex_init(&mutex, &mutex_attr);

	load();
}

Sensors::~Sensors() {
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_destroy(&mutex_attr);
}

int Sensors::startDaemon() {

	std::cout << "start daemon" << std::endl;

	set_max_priority();

	setupAlarm();

	MQTTClient mqttClient ("weather_sender");
	mqttClient.start();

	this->client = &mqttClient;

	Controller::instance().serverLoop();

	disableAlarm();

	this->client = NULL;

	mqttClient.stop();

	save();

	return 0;
}

void Sensors::load() {

	disableAlarm();
	pthread_mutex_lock(&mutex);

	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			delete sensors[channel];
			sensors[channel] = NULL;
		}
	}

	ifstream myfile;
	myfile.open(CONFIG_PATH CONFIG_FILE);

	string line;
	while (getline(myfile, line)) {
		if (line.at(0) == 'p') {
			pin = std::stoi( line.substr(1));
		}
		Sensor* next = new Sensor(pin);

		next->message(line.c_str());

		if (next->channel() >= 0 && next->channel() < sizeof(sensors)/sizeof(*sensors)) {
			sensors[next->channel()] = next;
		}
	}

	pthread_mutex_unlock(&mutex);
	enableAlarm();
}

void Sensors::save() {
	pthread_mutex_lock(&mutex);

	Utils::mkpath(CONFIG_PATH, 0700);

	ofstream config;
	config.open(CONFIG_PATH CONFIG_FILE);

	config << "p" << pin << endl;

	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			config << sensors[channel]->message() << endl;
		}
	}
	pthread_mutex_unlock(&mutex);
}

void Sensors::alarm() {
	pthread_mutex_lock(&mutex);
	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			cout << sensors[channel]->message() << endl;
			sensors[channel]->send();

			if (this->client != NULL) {
				this->client->sendStatus(channel + 1, "status", sensors[channel]->getStatusJson());
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}

UpdateCommand* Sensors::update(uint8_t channel) {
	return new LocalUpdateCommand(channel);
}

int Sensors::update(const UpdateCommand& cmd) {

	disableAlarm();
	pthread_mutex_lock(&mutex);

	Sensor* sensor = sensors[cmd._channel];

	if (sensor == NULL) {
		sensor = new Sensor(pin);
		sensor->channel(cmd._channel);
		sensors[cmd._channel] = sensor;
	}
	if (cmd._fields.test(FieldSet::Field::ID)) {
		sensor->id(cmd._id);
	}
	if (cmd._fields.test(FieldSet::Field::ALARM)) {
		sensor->alarm(cmd._alarm);
	}
	if (cmd._fields.test(FieldSet::Field::BATTERY)) {
		sensor->batteryOK(cmd._battery);
	}
	if (cmd._fields.test(FieldSet::Field::TEMPERATUE)) {
		sensor->temperature(cmd._temperature);
	}
	if (cmd._fields.test(FieldSet::Field::HUMIDITY)) {
		sensor->humidity(cmd._humidity);
	}

	pthread_mutex_unlock(&mutex);
	enableAlarm();

	save();

	return 0;
}

void Sensors::setupAlarm() {
	struct sigaction sa;
	struct itimerval timer;
	struct timeval now;

	gettimeofday(&now, 0);

	/* Installiere timer_handler als Signal Handler fuer SIGALRM. */
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &signal_handler;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	// start at timeofday % 57 == 0
	timer.it_value.tv_sec = 56 - (now.tv_sec % 57);
	timer.it_value.tv_usec = 1000000 - now.tv_usec;

	/* ... und alle 57s danach */
	timer.it_interval.tv_sec = 57;
	timer.it_interval.tv_usec = 0;

	/* Timer starten */
	setitimer(ITIMER_REAL, &timer, NULL);
}

void Sensors::disableAlarm() {
	sigset_t signal_set;
	sigemptyset(&signal_set);

	sigaddset(&signal_set, SIGALRM);
	sigaddset(&signal_set, SIGHUP);
	sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

void Sensors::enableAlarm() {
	sigset_t signal_set;
	sigemptyset(&signal_set);

	sigaddset(&signal_set, SIGALRM);
	sigaddset(&signal_set, SIGHUP);
	sigaddset(&signal_set, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

void set_max_priority(void) {
	struct sched_param sched;
	memset(&sched, 0, sizeof(sched));
	/* Use FIFO scheduler with highest priority for the
	 lowest chance of the kernel context switching. */
	sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(0, SCHED_FIFO, &sched);
}

void signal_handler(int signum) {
	switch (signum) {
	case SIGALRM:
//		cout << "Alarm" << endl;
		Sensors::instance().alarm();
		break;
	case SIGHUP:
//		cout << "Reload" << endl;
		Sensors::instance().load();
		break;
	case SIGTERM:
//		cout << "Exit" << endl;
		Controller::instance().stop();
	}
}
