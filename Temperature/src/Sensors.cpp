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


#define CONFIG_PATH "./"
#define CONFIG_FILE "config"
#define JSON_CONFIG_FILE "config.json"

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

	client.start();

	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			json status = *(sensors[channel]);
			this->client.sendStatus(channel + 1, "status", status.dump());
		}
	}

	//Controller::instance().serverLoop();

	disableAlarm();

	client.stop();

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

	json config;

	if (!doLoadJSON(config)) {
		doLoad(config);
	}

	pin = config.value("pin", pin);
	pinMode(pin, OUTPUT);

	const json::array_t sensorConf = config["sensors"];
	for (auto it = sensorConf.begin(); it != sensorConf.end(); ++it) {
		Sensor* s = new Sensor();
		*s = *it;
		json j = *s;
		cout << j.dump(4) << endl;
		sensors[s->channel()] = s;
	}

	client = config["mqtt"];

	pthread_mutex_unlock(&mutex);
	enableAlarm();
}

void Sensors::doLoad(json & config) {
	ifstream myfile;
	myfile.open(CONFIG_PATH CONFIG_FILE);

	config["sensors"] = json::array();

	string line;
	while (getline(myfile, line)) {
		if (line.at(0) == 'p') {
			int pin = std::stoi( line.substr(1));
			config["pin"] = pin;
			continue;
		}
		Sensor* next = new Sensor();

		next->message(line.c_str());

		if (next->channel() >= 0 && next->channel() < sizeof(sensors)/sizeof(*sensors)) {
			config["sensors"].push_back(line);
		}
	}

	cout << "read old config: " << config << endl;
}

bool Sensors::doLoadJSON(json & config) {
	ifstream myfile;
	myfile.open(CONFIG_PATH JSON_CONFIG_FILE);

	if (myfile.fail()) {
		return false;
	}

	myfile >> config;

	cout << "read new config: " << config << endl;

	return true;
}


void Sensors::save() {
	disableAlarm();
	pthread_mutex_lock(&mutex);

	doSave();

	pthread_mutex_unlock(&mutex);
	enableAlarm();
}

void Sensors::doSave() {
	Utils::mkpath(CONFIG_PATH, 0700);

	ofstream out;
	out.open(CONFIG_PATH JSON_CONFIG_FILE);

	json config;

	config["pin"] = pin;
	config["sensors"] = json::array();

	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			json status = *(sensors[channel]);
			config["sensors"].push_back(status);
		}
	}

	config["mqtt"] = client;

	out << config;

	cout << "write new config: " << config << endl;
}

void Sensors::alarm() {
	pthread_mutex_lock(&mutex);
	for (u_int8_t channel = 0; channel < sizeof(sensors)/sizeof(*sensors); channel++) {
		if (sensors[channel] != NULL) {
			sensors[channel]->send(pin);

			json status = *(sensors[channel]);
			this->client.sendStatus(channel + 1, "status", status.dump());
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
		sensor = new Sensor();
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

	json status = *sensor;
	this->client.sendStatus(cmd._channel + 1, "status", status.dump());

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
