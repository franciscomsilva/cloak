#include "ServerController.hpp"
#include "../modes/socks5/SocksProxyServer.hpp"

ServerController::ServerController(SocksProxyServer *sp){
    assert(sp != NULL);

    _sp = sp;
};

void ServerController::config(int client_port, int tor_port)
{
    _client_port = client_port;
    _tor_port = tor_port;
}


void ServerController::handleSocksNewConnection(FdPair *fdp)
{
    LOG(LogLevel::DEBUG, "Handling new connection (client: %d, tor: %d)",
             fdp->get_fd0(),fdp->get_fd1());
}

void ServerController::handleSocksRestoreConnection(FdPair *fdp) {

#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
    LOG(LogLevel::DEBUG, "Restored connection (client: %d, tor: %d)",
             fdp->get_fd0(),fdp->get_fd1());
#endif
}

void ServerController::handleSocksClientDataReady(FdPair *fdp)
{



    int nread, nwrite;
    char buffer_r[BUFSIZE];

    nread = _sp->readn_msg_client(fdp, buffer_r, BUFSIZE);
    if (nread <= 0) {
#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
        LOG(LogLevel::DEBUG, "[handleSocksClientDataReady] readn_msg_client - fd: %d - nread: %d - errno: %d ",fdp->get_fd0(),nread, errno );
#endif
        _sp->shutdown_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Read from client bytes (%d)", nread);

    nwrite = _sp->writen_msg_local(fdp, buffer_r, nread);
    if (nwrite <= 0) {
#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
        LOG(LogLevel::DEBUG, "[handleSocksClientDataReady] writen_msg_local - fd: %d - nwrite: %d - errno: %d ",fdp->get_fd1(),nwrite, errno );
#endif
        _sp->shutdown_local_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Wrote to tor bytes (%d)", nwrite);

}

void ServerController::handleSocksTorDataReady(FdPair *fdp)
{

    int nread, nwrite;
    char buffer_r[BUFSIZE];

    nread = _sp->readn_msg_local(fdp, buffer_r, BUFSIZE);
    if (nread <= 0) {
#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
        LOG(LogLevel::DEBUG, "[handleSocksTorDataReady] readn_msg_local - fd: %d - nread: %d - errno: %d ",fdp->get_fd1(),nread, errno );
#endif
        _sp->shutdown_local_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Read from tor bytes (%d)", nread);

    nwrite = _sp->writen_msg_client(fdp, buffer_r, nread);
    if (nwrite <= 0) {
#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
        LOG(LogLevel::DEBUG, "[handleSocksTorDataReady] writen_msg_client - fd: %d - nwrite: %d - errno: %d ",fdp->get_fd0(),nwrite, errno );
#endif
        _sp->shutdown_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Wrote to client bytes (%d)", nwrite);
}

void ServerController::handleSocksNewSessionError()
{
#if (LOG_VERBOSE & LOG_BIT_CONN & LOG_BIT_ERRORS)
    LOG(LogLevel::DEBUG, "Error establishing socks session");
#endif
}