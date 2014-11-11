#include <iostream>
#include <string>
#include <numeric>
#include <cmath>
#include <vector>
#include <functional>
#include <Windows.h>

double to_value(std::string::iterator& begin, const std::string::iterator& end) {
	auto count = 0;
	auto dot = -1;
	auto old = begin;

	auto sign = true;
	if(*begin == '+')
		++begin;
	else if(*begin == '-') {
		sign = false;
		++begin;
	}

	for(; begin < end; ++begin) {
		if(isdigit(*begin))
			++count;
		else if(*begin == '.' || *begin == ',')
			dot = begin - old;
		else
			break;
	}

	double ret = 0;
	auto offset = count - (dot == -1 ? count : dot) + 1;

	for(; count > 0; ++old) {
		if(isdigit(*old))
			ret += (double) (*old - '0') * pow(10, count-- - offset);
	}

	return ret * (sign ? 1 : -1);
}

double _calc(std::string::iterator& begin, const std::string::iterator& end) {
	std::vector<std::vector<double>> values;

	while(begin < end) {
		if(*begin == '(') {
			if(values.empty())
				values.push_back(std::vector<double>{_calc(++begin, end)});
			else
				values.back().push_back(_calc(++begin, end));
		} else if(*begin == ')') {
			++begin;
			break;
		} else if(*begin == '^') {
			if(begin + 1 < end && !values.empty() && !values.back().empty())
				values.back().back() = std::pow(values.back().back(), *(begin + 1) == '(' ? _calc(++begin, end) : to_value(++begin, end));
			else
				break;
		} else if(*begin == '*') {
			if(begin + 1 < end) {
				if(values.empty())
					values.push_back(std::vector<double>());
				values.back().push_back(*(begin + 1) == '(' ? _calc(++begin, end) : to_value(++begin, end));
			} else
				break;
		} else if(*begin == '/') {
			if(begin + 1 < end) {
				if(values.empty())
					values.push_back(std::vector<double>());
				values.back().push_back(1 / (*(begin + 1) == '(' ? _calc(++begin, end) : to_value(++begin, end)));
			} else
				break;
		} else if(*begin == '%') {
			if(begin + 1 < end) {
				if(values.empty())
					values.push_back(std::vector<double>());
				values.back().push_back(std::fmod(values.back().back(), *(begin + 1) == '(' ? _calc(++begin, end) : to_value(++begin, end)));
			} else
				break;
		} else if(*begin == '+' || *begin == '-') {
			if(begin + 1 < end)
				values.push_back(std::vector<double>{ *(begin + 1) == '(' ? _calc(++begin, end) : to_value(begin, end) });
			else
				break;
		} else if(isdigit(*begin)) {
			if(values.empty())
				values.push_back(std::vector<double>{ to_value(begin, end) });
			else
				values.back().push_back(to_value(begin, end));
		} else
			++begin;
	}

	return std::accumulate(values.begin(), values.end(), 0.0, [](double a, std::vector<double> b)->double {
		return a + std::accumulate(b.begin(), b.end(), 1.0, std::multiplies<double>());
	});
}

double calc(std::string math) {
	auto begin = math.begin();
	return _calc(begin, math.end());
}

class ConsoleHelper {
public:
	static char get_char_async() {
		auto hConsole = GetStdHandle(STD_INPUT_HANDLE);
		DWORD oldModeValue;
		GetConsoleMode(hConsole, &oldModeValue);
		SetConsoleMode(hConsole, oldModeValue & ~ENABLE_LINE_INPUT);
		char out;
		ReadFile(hConsole, &out, 1, 0, 0);
		SetConsoleMode(hConsole, oldModeValue);
		return out;
	}

	static std::pair<short, short> get_position() {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		return{ csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y };
	}

	static void set_position(short x, short y) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
	}
};

int main() {
	std::string buffer;
	while(1) {
		auto chr = ConsoleHelper::get_char_async();
		if(chr == '\b') {
			if(!buffer.empty()) {
				buffer.pop_back();
				auto pos = ConsoleHelper::get_position();
				std::cout << "\b \b";
			}
		} else if(buffer.length() < 79) {
			buffer += chr;
			std::cout << chr;
		}
		auto pos = ConsoleHelper::get_position();
		ConsoleHelper::set_position(0, pos.second + 1);
		std::cout << '=' << calc(buffer) << "        ";
		ConsoleHelper::set_position(pos.first, pos.second);
	}
}