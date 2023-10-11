#include "Common.hpp"
#include<stdarg.h>


/* ================================ Functions ============================= */
int readn(int fd, void *buf, int n)
{
    int nread, left = n;
    while (left > 0) {
        if ((nread = read(fd, buf, left)) == -1) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return nread;
        } else {
            if (nread == 0) {
                return 0;
            } else {
                left -= nread;
                buf = (char*) buf + nread;
            }
        }
    }
    return n;
}

int writen(int fd, void *buf, int n)
{
    int nwrite, left = n;
    while (left > 0) {
        if ((nwrite = write(fd, buf, left)) == -1) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return nwrite;
        } else {
            if (nwrite == n) {
                return n;
            } else {
                left -= nwrite;
                buf = (char*) buf + nwrite;
            }
        }
    }
    return n;
}

/* Logging functions and macros*/

// Global log level variable
LogLevel IPC_LOG_LEVEL= LogLevel::NONE;

void Logger(LogLevel level, const char* message, ...) {

    // If global log level does not match, do nothing
    if(level > IPC_LOG_LEVEL || IPC_LOG_LEVEL == LogLevel::NONE)
        return;

    std::string prefix;
    char vbuffer[255];
    va_list args;
    va_start(args, message);
    vsnprintf(vbuffer, ARRAY_SIZE(vbuffer), message, args);
    va_end(args);

    switch (level) {
        case LogLevel::ERROR: prefix = "ERROR "; break;
        case LogLevel::WARN: prefix = "WARNING "; break;
        case LogLevel::INFO: prefix = "INFO "; break;
        case LogLevel::DEBUG: prefix = "DEBUG "; break;
    }

    std::cout << "LOG " << prefix << vbuffer << std::endl;
}