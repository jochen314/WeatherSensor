#include "Sensor.hpp"

#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>

#define START_LEN 4000
#define GAP_LEN 550
#define ONE_LEN 1840
#define ZERO_LEN 920

char *setBits(char *message, int nibble, int count) {
	int mask = (1 << (count - 1));

	while (mask != 0) {
		*(message++) = nibble & mask ? '1' : '0';
		mask >>= 1;
	}
	return message;
}

int getBits(const char *message, int count) {
	int result = 0;
	int mask = (1 << (count - 1));

	while (mask != 0 && *message) {
		switch (*(message++)) {
		case '0':
			break;
		default:
			result |= mask;
			break;
		}
		mask >>= 1;
	}
	return result;
}

Sensor::Sensor() {
	memset(_message, '0', 36);
	_message[37] = 0;

	setBits(_message + 24, 0xF, 4);
}

Sensor& Sensor::id(uint8_t id) {
	setBits(_message, id, 8);
	return *this;
}

Sensor& Sensor::alarm(bool flag) {
	setBits(_message + 9, flag, 1);
	return *this;
}

Sensor& Sensor::batteryOK(bool flag) {
	setBits(_message + 8, flag, 1);
	return *this;
}

Sensor& Sensor::channel(uint8_t channel) {
	setBits(_message + 10, channel, 2);
	return *this;
}

Sensor& Sensor::temperature(int temperature) {
	setBits(_message + 12, temperature, 12);
	return *this;
}

Sensor& Sensor::humidity(int percent) {
	setBits(_message + 28, percent, 8);
	return *this;
}

Sensor& Sensor::message(const char * message) {
	// TODO check message values

	strncpy(_message, message, sizeof(_message));
	return *this;
}

const char * Sensor::message() const {
	return _message;
}

uint8_t Sensor::id() const {
	return (uint8_t) getBits(_message, 8);
}

bool Sensor::alarm() const {
	return (bool) getBits(_message + 9, 1);
}

bool Sensor::batteryOK() const {
	return (bool) getBits(_message + 8, 1);
}

uint8_t Sensor::channel() const {
	return (uint8_t) getBits(_message + 10, 2);
}

float Sensor::temperature() const {
	int16_t temp = getBits(_message + 12, 12);
	if (temp & (1 << 11)) {
		temp |= 0xf000;
	}

	return (float) temp / 10;
}

int Sensor::humidity() const {
	return getBits(_message + 28, 8);
}

void sendBit(int dataPin, long lowTime, long highTime) {
	digitalWrite(dataPin, HIGH);
	delayMicroseconds(highTime);
	digitalWrite(dataPin, LOW);
	delayMicroseconds(lowTime);
}

void sendMessage(int dataPin, const char *message) {
	sendBit(dataPin, START_LEN, GAP_LEN);
	while (*message) {
		switch (*(message++)) {
		case '0':
			sendBit(dataPin, ZERO_LEN, GAP_LEN);
			break;
		default:
			sendBit(dataPin, ONE_LEN, GAP_LEN);
			break;
		}
	}
	digitalWrite(dataPin, LOW);
}

void Sensor::send(int dataPin) const {
	for (int i = 0; i < 12; i++) {
		sendMessage(dataPin, _message);
	}
}

void to_json(json& j, const Sensor& s) {
	j = json { { "channel", 1 + s.channel() }, { "id", s.id() }, { "alarm",
			s.alarm() }, { "battery", s.batteryOK() }, { "temperature",
			s.temperature() }, { "humidity", s.humidity() } };
}

void from_json(const json& j, Sensor& s) {

	switch (j.type()) {
	case json::value_t::string:
		s.message(j.get<std::string>().c_str());
		break;
	case json::value_t::object: {
		auto it = j.find("channel");
		if (it != j.end()) {
			s.channel(it->get<int>() - 1);
		}
		it = j.find("id");
		if (it != j.end()) {
			s.id(it->get<int>());
		}
		it = j.find("alarm");
		if (it != j.end()) {
			s.alarm(it->get<bool>());
		}
		it = j.find("battery");
		if (it != j.end()) {
			s.batteryOK(it->get<int>());
		}
		it = j.find("temperature");
		if (it != j.end()) {
			s.temperature(it->get<int>());
		}
		it = j.find("humidity");
		if (it != j.end()) {
			s.humidity(it->get<int>());
		}
	}
		break;
	default:
		break;
	}
}
