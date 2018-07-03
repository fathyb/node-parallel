//
// Created by fathy on 04/06/2018.
//

#ifndef NODE_SHARED_BUFFER_LOGGER_H
#define NODE_SHARED_BUFFER_LOGGER_H


#include <iostream>
#include <sstream>

namespace nsb {
	enum LogLevel {
		Error, Warning, Info, Debug
	};

	class Logger {
	public:
		explicit Logger(LogLevel level = Error);
		~Logger();

		template <typename T>
		Logger& operator<<(T const& value) {
			buffer << value;

			return *this;
		}

		static LogLevel logLevel;
	private:
		std::ostringstream buffer;
	};
}

#define log(level) \
	if (level > nsb::Logger::logLevel) ; \
	else nsb::Logger(level)

#endif //NODE_SHARED_BUFFER_LOGGER_H
