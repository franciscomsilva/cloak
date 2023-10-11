#include "ClientController.hpp"
#include "../modes/socks5/SocksProxyClient.hpp"



ClientController::ClientController(SocksProxyClient *sp){
    assert(sp != NULL);

    _sp = sp;
};

void ClientController::config(int socks_port, int local_port)
{
    _socks_port = socks_port;
    _local_port = local_port;
}

void ClientController::handleSocksNewConnection(FdPair *fdp)
{

    LOG(LogLevel::DEBUG, "Handling new connection (client: %d, bridge: %d)",
    fdp->get_fd0(),fdp->get_fd1());

}

void ClientController::handleSocksRestoreConnection(FdPair *fdp) {

#if (LOG_VERBOSE & LOG_BIT_CTRL_LOCK)
    LOG(LogLevel::DEBUG, "Restored connection (client: %d, bridge: %d)",
             fdp->get_fd0(),fdp->get_fd1());
#endif
}

void ClientController::handleSocksClientDataReady(FdPair *fdp)
{
    int nread, nwrite;
    char buffer_r[BUFSIZE];

    nread = _sp->readn_msg_client(fdp, buffer_r, BUFSIZE);
    if (nread <= 0) {
        LOG(LogLevel::DEBUG, "[handleSocksClientDataReady] readn_msg_client - fd: %d - nread: %d - errno: %d ",fdp->get_fd0(),nread, errno );
        _sp->shutdown_client_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Read from client bytes (%d)", nread);

    nwrite = _sp->writen_msg_bridge(fdp, buffer_r, nread);
    if (nwrite <= 0) {
         _sp->shutdown_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Wrote to bridge bytes (%d)", nwrite);

}

void ClientController::handleSocksBridgeDataReady(FdPair *fdp)
{
    int nread, nwrite;
    char buffer_r[BUFSIZE];

    nread = _sp->readn_msg_bridge(fdp, buffer_r, BUFSIZE);
    if (nread <= 0) {
        _sp->shutdown_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Read from bridge bytes (%d)", nread);

    nwrite = _sp->writen_msg_client(fdp, buffer_r, nread);
    if (nwrite <= 0) {
        LOG(LogLevel::DEBUG, "[handleSocksBridgeDataReady] writen_msg_client - fd: %d - nwrite: %d - errno: %d ",fdp->get_fd0(),nwrite, errno );

        _sp->shutdown_client_connection(fdp);
        return;
    }

    LOG(LogLevel::DEBUG, "Wrote to client bytes (%d)", nwrite);

}

void ClientController::handleSocksNewSessionError()
{
#if (LOG_VERBOSE & LOG_BIT_CONN & LOG_BIT_ERRORS)
    LOG(LogLevel::DEBUG, "Error establishing socks session");
#endif
}

void ClientController::handleSocksConnectionTerminated(FdPair *fdp)
{
    LOG(LogLevel::DEBUG, "Terminated connection (client: %d, bridge: %d)",
             fdp->get_fd0(), fdp->get_fd1());
}


