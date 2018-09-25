
#pragma once

#include <string>
#include <functional>
#include <sstream>

/**
 * Provides a application wide way to log data for debug and traceability
 */
class Logger
{
public:
	/**
	 * The log levels
	 */
	enum LogLevel
	{
		DEBUG = 0,
		INFO = 1,
		WARNING = 2,
		ERROR = 3,
        NONE
	};

	static void setMinLogLevel(Logger::LogLevel level) { Logger::_minLogLevel = level; }
	static Logger::LogLevel getMinLogLevel() { return Logger::_minLogLevel; }

	/**
	 * Set the log handler which all data from @see Log() calls gets forwarded to
	 * @param callback The callback to store or show the log-data
	 */
	static void setLogHandler(std::function<void(LogLevel, std::string)> callback) { _callback = callback; }
	/**
	 * Log a message with a given log-level
	 * @param level The log-level for this message
	 * @param arg The first argument to log
	 * @param args Optional other arguments to log
	 */
	template <typename Arg, typename... Args>
	static void Log(Logger::LogLevel level, Arg&& arg, Args&&... args)
	{
//#ifdef NDEBUG
//			return;
//#endif
		if(level < _minLogLevel)
			return;
		
		std::ostringstream tmpStream;

		tmpStream << std::forward<Arg>(arg);
		using expander = int[];
		(void)expander{0, (static_cast<void>(void(tmpStream << std::forward<Args>(args))),0)...};

		_callback(level, tmpStream.str());
	}

private:
	/**
	 * A handle to the function which will store or print the logged data
	 */
	static std::function<void(LogLevel, std::string)> _callback;
	/**
	 * A helper so we can stringify the LogLevel Enum
	 */
	static std::string consoleTranslation[];
	/**
	 * A default implementation of a callback. Will print all logged data to the console
	 * @param level The log level
	 * @param value The message to log
	 */
	static void defaultCallback(LogLevel level, std::string value);
	static LogLevel _minLogLevel;
};
