#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <ostream>
#include <string>
#include <sstream>

#include <QTime>

#if __GNUC__
#include <sys/time.h>
#endif

// Comment this line if you don't need multithread support

#if __GNUC__
#define LOGGER_MULTITHREAD
#endif
#define LOGGER_FILE_DEBUG
const int DBG_ERROR	= 0;
const int DBG_WARN	= 1;
const int DBG_DEBUG	= 2;

#define LOGGER_PRINT_FILE_LINE
//#undef LOGGER_PRINT_FILE_LINE


#ifdef LOGGER_MULTITHREAD
#include <pthread.h>
#endif


/**
 * \brief Macro to configure the logger.
 * Example of configuration of the Logger:
 * 	DEBUG_CONF("outputfile", Logger::file_on|Logger::screen_on, DBG_DEBUG, DBG_ERROR);
 */
#define DEBUG_CONF(outputFile, \
        configuration, \
        fileVerbosityLevel, \
        screenVerbosityLevel) { \
            Logger::getInstance().configure(outputFile, \
                        configuration, \
                        fileVerbosityLevel, \
                        screenVerbosityLevel); \
        }

/**
 * \brief Macro to print log messages.
 * Example of usage of the Logger:
 *	    DEBUG(DBG_DEBUG, "hello " << "world");
 */
#if 1
#define DEBUG(priority, msg) { \
    std::ostringstream __debug_stream__; \
    __debug_stream__ << msg; \
    Logger::getInstance().print(priority,\
            __FILE__, __LINE__,\
            __debug_stream__.str()); \
    }
#else
#define DEBUG(priority, msg) { \
    std::ostringstream __debug_stream__; \
    __debug_stream__ << msg; \
    Logger::getInstance().print(priority, __FILE__, __LINE__, \
            __debug_stream__.str()); \
    }
#endif





/**
 * \brief Simple logger to log messages on file and console.
 * This is the implementation of a simple logger in C++. It is implemented
 * as a Singleton, so it can be easily called through two DEBUG macros.
 * It is Pthread-safe.
 * It allows to log on both file and screen, and to specify a verbosity
 * threshold for both of them.
 */
class Logger
{
    /**
     * \brief Type used for the configuration
     */
    enum loggerConf_	{L_nofile_	= 	1 << 0,
                L_file_		=	1 << 1,
                L_noscreen_	=	1 << 2,
                L_screen_	=	1 << 3};

#ifdef LOGGER_MULTITHREAD
    /**
     * \brief Lock for mutual exclusion between different threads
     */
    static pthread_mutex_t lock_;
#endif

    bool configured_;

    /**
     * \brief Pointer to the unique Logger (i.e., Singleton)
     */
    static Logger* m_;

    /**
     * \brief Initial part of the name of the file used for Logging.
     * Date and time are automatically appended.
     */
    std::string logFile_;

    /**
     * \brief Current configuration of the logger.
     * Variable to know if logging on file and on screen are enabled.
     * Note that if the log on file is enabled, it means that the
     * logger has been already configured, therefore the stream is
     * already open.
     */
    loggerConf_ configuration_;

    /**
     * \brief Stream used when logging on a file
     */
    std::ofstream out_;

    /**
     * \brief Initial time (used to print relative times)
     */
#if __GNUC__
    struct timeval initialTime_;
#endif
    QDateTime _initialQDateTime;

    /**
     * \brief Verbosity threshold for files
     */
    unsigned int fileVerbosityLevel_;

    /**
     * \brief Verbosity threshold for screen
     */
    unsigned int screenVerbosityLevel_;

    Logger();
    ~Logger();

    /**
     * \brief Method to lock in case of multithreading
     */
    inline static void lock();

    /**
     * \brief Method to unlock in case of multithreading
     */
    inline static void unlock();

public:

    typedef loggerConf_ loggerConf;
    static const loggerConf file_on= 	L_nofile_;
    static const loggerConf file_off= 	L_file_;
    static const loggerConf screen_on= 	L_noscreen_;
    static const loggerConf screen_off= L_screen_;

    static Logger& getInstance();

    void print(const unsigned int verbosityLevel,
                        const std::string& file,
                        const int line,
                        const std::string& message);

    void configure (const std::string&	outputFile,
            const loggerConf	configuration,
            const int		fileVerbosityLevel,
            const int		screenVerbosityLevel);

    const char * getLogFilename()
    {
        return logFile_.c_str();
    }
};

inline Logger::loggerConf operator|
    (Logger::loggerConf __a, Logger::loggerConf __b)
{
    return Logger::loggerConf(static_cast<int>(__a) |
        static_cast<int>(__b));
}

inline Logger::loggerConf operator&
    (Logger::loggerConf __a, Logger::loggerConf __b)
{
    return Logger::loggerConf(static_cast<int>(__a) &
        static_cast<int>(__b)); }

#endif // LOGGER_H
