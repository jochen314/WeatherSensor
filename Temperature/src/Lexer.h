/*
 * Lexer.h
 *
 *  Created on: 10.04.2018
 *      Author: jochen
 */

#ifndef LEXER_H_
#define LEXER_H_

#include <string>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <regex>

enum class TokenType {
	DAEMON,
	START,
	STOP,
	RELOAD,
	CHANNEL,
	ID,
	ALARM,
	ON,
	OFF,
	BATTERY,
	OK,
	BAD,
	TEMPERATURE,
	HUMIDITY,

	INTEGER,
	FLOAT,

	OTHER
};

class Lexer;

class Token {
private:
	TokenType _type;
	std::string _str;
	int _int;
	float _float;

protected:
	friend class Lexer;
	Token(TokenType type, std::string value) : _type(type), _str(value), _int(), _float() {}
	Token(int value) : _type(TokenType::INTEGER), _str(), _int(value), _float()  {}
	Token(float value) : _type(TokenType::FLOAT), _str(), _int(), _float(value)  {}
public:
	TokenType type() {
		return _type;
	}

	explicit operator std::string() {
		switch (_type) {
		case TokenType::INTEGER:
			return std::to_string(_int);
		case TokenType::FLOAT:
			return std::to_string(_float);
		default:
			return _str;
		}
	}

	explicit operator int() {
		switch (_type) {
		case TokenType::INTEGER:
			return _int;
		case TokenType::FLOAT:
			return nearbyint(_float);
		default:
			throw std::invalid_argument("not a number");
		}
	}

	explicit operator float() {
		switch (_type) {
		case TokenType::INTEGER:
			return _int;
		case TokenType::FLOAT:
			return _float;
		default:
			throw std::invalid_argument("not a number");
		}
	}
};

float operator * (float f1, Token& f2);

class Lexer {
private:
	std::unordered_map<std::string, TokenType> words;
	std::regex reg_integer;
	std::regex reg_float;

	Lexer();
	virtual ~Lexer();
public:
	static Lexer& instance();
	Token toToken(std::string word);
};

#endif /* LEXER_H_ */
