#include "common/cmdline.h"
#include "torptspec/TorPT.hpp"
#include "torptspec/TorPTClient.hpp"
#include "torptspec/TorPTServer.hpp"
#include "transport/Transport.h"
#include "modes/socks5/SocksProxyClient.hpp"
#include "modes/socks5/SocksProxyServer.hpp"


pid_t socksPID;
Parameters p;

void gracefullyTerminate() {
    exit(0);
}

void local_terminate_handler(int signum)
{
    switch(signum){
        case SIGKILL:
            LOG(LogLevel::WARN, "Received SIGKILL. Terminating");
            break;

        case SIGTERM:
            LOG(LogLevel::WARN, "Received SIGTERM. Terminating");
            break;

        case SIGINT:
            LOG(LogLevel::WARN,"Received SIGINT. Terminating");
            break;

        case 1:
            LOG(LogLevel::WARN,"Tor closed stdin. Terminating");
            break;
    }
    gracefullyTerminate();
}

void parse_args(int argc, char* argv[], Parameters &p)
{
    cmdline::parser parser;

    // Defines the required arguments for the PT 3.0 Specification

    // Common configuration parameters
    parser.add<std::string>("ptversion", '\0', "PT Specification Version", false, "1");
    parser.add<std::string>("state", '\0', "Path to a  directory where the PT is allowed to store state that will be persisted across invocations", false, ".");
    parser.add("exit-on-stdin-close", '\0', "Specifies that the parent process will close the PT proxy's standard input (stdin)");
    parser.add<std::string>("ipcLogLevel", '\0', "Controls what log messages are sent from the dispatcher to the application using LOG messages", false, "NONE");
    parser.add<std::string>("transport", '\0', "Specifies the name of the PT to use", false, "");
    parser.add<std::string>("optionsFile", '\0', "Specifies the path to a file containing the transport options", false, "");
    parser.add<std::string>("transports", '\0', "Specifies the PT protocols the client/server proxy should initialize, as a comma separated list of PT names", false, "");
    parser.add<std::string>("mode", '\0', "Sets the proxy mode", false, "socks5");

    // Client configuration parameters
    parser.add("client", '\0', "Specifies that the PT proxy should run in client mode");
    parser.add<std::string>("proxylistenaddr", '\0', "Specifies the <address>:<port> on which the dispatcher client should listen for incoming application client connections", false, "");
    parser.add<std::string>("proxylistenhost", '\0', "Specifies the <address> on which the dispatcher client should listen for incoming application client connections", false, "127.0.0.1");
    parser.add<int>("proxylistenport", '\0', "Specifies the <port> on which the dispatcher client should listen for incoming application client connections", false, 1080);
    parser.add<std::string>("proxy", '\0', "Specifies an upstream proxy that the PT must use when making outgoing network connections", false, "");


    // Server configuration parameters
    parser.add("server", '\0', "Specifies that the PT proxy should run in server mode");
    parser.add<std::string>("options", '\0', "Specifies per-PT protocol configuration directives, as a JSON string value with options that are to be passed to the transport", false, "");
    parser.add<std::string>("bindaddr", '\0', "A comma separated list of <key>-<value> pairs, where <key> is a PT name and <value> is the <address>:<port> on which it should listen for incoming client connections", false, "");
    parser.add<std::string>("bindhost", '\0', "Specifies the <address> part of the server bind address when used in conjunction with -transport and -bindport", false, "");
    parser.add<int>("bindport", '\0', "Specifies the <port> part of the server bind address when used in conjunction with -transport and -bindhost", false, 0);
    parser.add<std::string>("target", '\0', "Specifies the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate, as an <address>:<port>", false, "");
    parser.add<std::string>("targethost", '\0', "Specifies the <address> of the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate", false, "");
    parser.add<int>("targetport", '\0', "Specifies the <port> of the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate", false, 0);
    parser.add<std::string>("extorport", '\0', "Specifies the destination that the PT reverse proxy should forward traffic to, via the Extended ORPort protocol [EXTORPORT] as an <address>:<port>", false, "");
    parser.add<std::string>("authcookie", '\0', "Specifies an absolute filesystem path to the Extended ORPort authentication cookie", false, "");    

    parser.parse_check(argc, argv);

    // Stores parameters values
    p.mode         = parser.get<std::string>("mode");
    p.ptversion    = parser.get<std::string>("ptversion");
    p.state        = parser.get<std::string>("state");
    p.ipcLogLevel  = parser.get<std::string>("ipcLogLevel");
    p.transport    = parser.get<std::string>("transport");
    p.optionsFile  = parser.get<std::string>("optionsFile");
    p.transports   = parser.get<std::string>("transports");
    p.exit_on_stdin_close = parser.exist("exit-on-stdin-close");
    
    p.proxylistenaddr = parser.get<std::string>("proxylistenaddr");
    p.proxylistenhost = parser.get<std::string>("proxylistenhost");
    p.proxylistenport = parser.get<int>("proxylistenport");
    p.proxy           = parser.get<std::string>("proxy");

    p.options      = parser.get<std::string>("options");
    p.bindaddr     = parser.get<std::string>("bindaddr");
    p.bindhost     = parser.get<std::string>("bindhost");
    p.bindport     = parser.get<int>("bindport");
    p.target       = parser.get<std::string>("target");
    p.targethost   = parser.get<std::string>("targethost");
    p.targetport   = parser.get<int>("targetport");
    p.extorport    = parser.get<std::string>("extorport");
    p.authcookie   = parser.get<std::string>("authcookie");
    p.client       = parser.exist("client");
    p.server       = parser.exist("server");

}



int main(int argc, char* argv[])
{
    /* Sets the behavior for received signals*/
    std::signal(SIGINT, local_terminate_handler);
    std::signal(SIGTERM, local_terminate_handler);
    std::signal(SIGKILL, local_terminate_handler);



    // Parses the required arguments
    parse_args(argc, argv, p);

    // Client mode    
    if(p.client && !p.server || !p.client && !p.server){
        TorPTClient pt;
        SocksProxyClient proxy;
        Transport transport;
        ClientController controller(&proxy);

        pt.initialize(&p,RUN_FOREGROUND);

        controller.config(pt.getProxyPort(), 2080);

        transport.initialize("client", controller.get_local_port());

        proxy.initialize(&controller,false, INV_FD, RUN_BACKGROUND);

    }     
    // Server mode   
    else if(p.server && !p.client)
    {   
        TorPTServer pt;
        SocksProxyServer proxy;
        Transport transport;
        ServerController  controller(&proxy);

        pt.initialize(&p,RUN_FOREGROUND);

        controller.config(pt.getBindPort(), pt.getTargetPort() );

        proxy.initialize(&controller, RUN_BACKGROUND);

        transport.initialize("server", controller.get_client_port());


    }
    else{
        std::cerr << "Fatal: Choose -client or -server mode! " << std::endl;
        return 1;
    }

    std::string stdin_line;
    while (std::getline(std::cin, stdin_line));

    gracefullyTerminate();

    return 0;

}
