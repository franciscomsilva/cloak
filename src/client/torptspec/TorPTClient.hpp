#ifndef TORPTCLIENT_H
#define TORPTCLIENT_H

#include <assert.h>
#include "TorPT.hpp"
#include "../common/Common.hpp"


//Tor Vars
#define PT_CLIENT_TRANSPORTS "-transports"
#define PT_PROXY             "-proxy"
#define PT_LISTENADDR_LISTENHOST_LISTENPORT "-listenaddr & -listenhost || -listenport"
#define PT_LISTENHOST_LISTENPORT "-listenhost -listenport"


class TorPTClient : public TorPT {

    public:

        TorPTClient(){};

        void initialize(Parameters *p, int run_mode);

        int getProxyPort() { return  std::stoi(pt_proxy_listen_port);}


protected:

        inline void checkIfVarsAreSet(Parameters *p);

        void parseVariables(Parameters *p);

        void notifyParentAboutProxy(short proxy_status);

    private:

        /* -transports
        
            Specifies the PT protocols the client proxy should initialize, as a comma separated list of PT names.

            PTs SHOULD ignore PT names that it does not recognize.
        */
        std::vector<std::string> pt_transports;
        
        /*
            -proxylistenaddr 

            This flag specifies the <address>:<port> on which the dispatcher client should listen for incoming application 
            client connections. When this flag is used, the dispatcher client will use this address and port instead of making
            its own choice.

            The <address>:<port> combination MUST be an IP address supported by bind(), and MUST NOT be a host name.
        */
        std::string pt_proxy_listen_addr = "127.0.0.1:1080";

         /*
            -proxylistenhost

            This flag specifies the on which the dispatcher client should listen for incoming application client connections. 
            When this flag is used, the dispatcher client will use this address instead of making its own choice.
            The combination MUST be an IP address supported by bind(), and MUST NOT be a host name.

        */

        std::string pt_proxy_listen_host = "127.0.0.1";

         /*
            -proxylistenport

            This flag specifies the <port> on which the dispatcher client should listen for incoming application client connections.
            When this flag is used, the dispatcher client will use this port instead of making its own choice.

            The <address>:<port> combination MUST be an IP address supported by bind(), and MUST NOT be a host name.

        */

        std::string pt_proxy_listen_port = "1080";

        /*
            -proxy

            Specifies an upstream proxy that the PT MUST use when making outgoing network connections. It is a URI [RFC3986] of the format:

            <proxy_type>://[<user_name>[:<password>][@]<ip>:<port>.
        */
        
        std::string pt_upstream_proxy;

        
};

#endif // TORPTCLIENT_H