#ifndef FDPAIR_HH
#define FDPAIR_HH

#include "Common.hpp"

#define INV_FD (-1)

class FdPair {

public:

    FdPair(int fd0, int fd1): _fd0(fd0), _fd1(fd1) {};


    ~FdPair() {}

    int get_fd0() {
        return _fd0;
    }

    int get_fd1() {
        return _fd1;
    }

    void set_fd0(int fd0) {
        _fd0 = fd0;
    }

    void set_fd1(int fd1) {
        _fd1 = fd1;
    }

private:

    int _fd0;
    int _fd1;


};

#endif //FDPAIR_HH
