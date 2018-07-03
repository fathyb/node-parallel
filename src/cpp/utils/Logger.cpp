//
// Created by fathy on 04/06/2018.
//

#import "Logger.h"
#include "../platform/Platform.h"

using namespace nsb;

LogLevel Logger::logLevel = Debug;

namespace {
	std::string GetLevelName(LogLevel level) {
		size_t neededLength = 8;
		std::string name = "[";

		switch(level) {
			case Debug:
				name = "Debug";
				break;
			case Info:
				name = "Info";
				break;
			case Warning:
				name = "Warning";
				break;
			case Error:
				name = "Error";
		}

		int bit = false;
		while(name.size() != neededLength) {
			bit = !bit;

			if(bit) {
				name += " ";
			}
			else {
				name.insert(0, " ");
			}
		}

		return name;
	}
}

nsb::Logger::Logger(nsb::LogLevel level) {
	buffer << "[" << platform::GetProcessId() << "] [ " << GetLevelName(level) << "] :"
			<< std::string(
				level > Debug
					? (level - Debug) * 4
					: 1,
				' '
			);
}

Logger::~Logger() {
	fprintf(stderr, "%s\n", buffer.str().c_str());
}
