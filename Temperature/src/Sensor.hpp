#ifndef _Sensor_h
#define _Sensor_h

#include <stdint.h>
#include <wiringPi.h>

class Sensor{
  public:
    Sensor(int dataPin);

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

    void send() const;

	const char* message() const;
	Sensor& message(const char* message);

  private:
    int _dataPin;
    char _message[37];
};


#endif