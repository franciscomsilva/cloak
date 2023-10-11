#ifndef COMMON_HPP
#define COMMON_HPP

#define RUN_FOREGROUND (0)
#define RUN_BACKGROUND (1)

/* ============================== Logging Options =========================== */

/* None = 0, Error = 1, Warn = 2, Info = 3, Debug = 4 */
enum class LogLevel{
    NONE,
    ERROR,
    WARN,
    INFO,
    DEBUG
};

// Global loglevel variable
extern LogLevel IPC_LOG_LEVEL;

// Logging function
void Logger(LogLevel level, const char* message, ...);

// Logging macro
#define LOG(level, ...) Logger(level, __VA_ARGS__)

/* =============================== Connections ============================ */

/* Maximum number of pending connections on the listen socket file descriptor.*/
#define MAX_LISTEN_USERS (50)

/* ============================ Proxy Modes ========================== */
#define TRANSPARENT_TCP 1
#define TRANSPARENT_UDP 2
#define SOCKS5          3
#define STUN            4

/* ============================ General Definitions ========================== */


#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define ARRAY_INIT    {0}
#define BUFSIZE (2048)


/* ================================ Libraries ============================= */

#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <shared_mutex>
#include <condition_variable>
#include <sstream>  
#include <filesystem>
#include <fstream>
#include <regex>
#include <fcntl.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


/* ================================ Functions ============================= */

int readn(int fd, void *buf, int n);

int writen(int fd, void *buf, int n);

void log(const char *message, ...);

#endif //COMMON_HPP