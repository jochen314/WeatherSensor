/*
 * Lexer.cpp
 *
 *  Created on: 10.04.2018
 *      Author: jochen
 */

#include "Lexer.h"

float operator * (float f1, Token& f2) {
	return f1 * (float)f2;
}

Lexer& Lexer::instance() {
	static Lexer _instance;
	return _instance;
}


Lexer::Lexer(): reg_integer ("(\\+|-)?[[:digit:]]+"), reg_float("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?") {
	words["daemon"] = TokenType::DAEMON;
	words["start"] = TokenType::START;
	words["stop"] = TokenType::STOP;
	words["reload"] = TokenType::RELOAD;
	words["channel"] = TokenType::CHANNEL;
	words["id"] = TokenType::ID;
	words["alarm"] = TokenType::ALARM;
	words["on"] = TokenType::ON;
	words["off"] = TokenType::OFF;
	words["battery"] = TokenType::BATTERY;
	words["ok"] = TokenType::OK;
	words["bad"] = TokenType::BAD;
	words["temperature"] = TokenType::TEMPERATURE;
	words["humidity"] = TokenType::HUMIDITY;
}

Lexer::~Lexer() {
}



Token Lexer::toToken(std::string word) {
	if (std::regex_match(word, reg_integer)) {
		return Token(std::stoi(word));
	}
	if (std::regex_match(word, reg_float)) {
		return Token((float)std::stod(word));
	}
	if (words.count(word) == 0) {
		return Token(TokenType::OTHER, word);
	}
	return Token(words[word], word);

}
