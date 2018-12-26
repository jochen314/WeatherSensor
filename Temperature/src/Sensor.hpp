#ifndef _Sensor_h
#define _Sensor_h

#include <stdint.h>
#include <string>
#include <wiringPi.h>
#include <json.hpp>
using json = nlohmann::json;

class Sensor{
  public:
    Sensor();

    Sensor& id(uint8_t id);
    Sensor& alarm(bool flag);
    Sensor& batteryOK(bool flag);
    Sensor& channel(uint8_t channel);
    Sensor& temperature(int temperature);
    Sensor& humidity(int percent);

    uint8_t id() const;
    bool alarm() const;
    bool batteryOK() const;
    uint8_t channel() const;
    float temperature() const;
    int humidity() const;

    void send(int dataPin) const;

	const char* message() const;
	Sensor& message(const char* message);
  private:
    char _message[37];
};

void to_json(json& j, const Sensor& s) ;
void from_json(const json& j, Sensor& s) ;

#endif
