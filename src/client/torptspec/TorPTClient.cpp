#include "TorPTClient.hpp"



void TorPTClient::initialize(Parameters *p, int run_mode)
{
    assert(run_mode == RUN_FOREGROUND);

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Parsing vars...\"");
    parseVariables(p);
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Starting Proxy...\"");

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Proxy is running...\"");
    notifyParentAboutProxy(0);

}

void TorPTClient::checkIfVarsAreSet(Parameters *p) {
    if(p->proxylistenaddr != "" && (p->proxylistenhost != "" || p->proxylistenport != 0)){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_LISTENADDR_LISTENHOST_LISTENPORT);
        exit(1);
    }

    if((p->proxylistenhost != "" && p->proxylistenport == 0 )|| (p->proxylistenhost == "" && p->proxylistenport != 0 )){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_LISTENHOST_LISTENPORT);
        exit(1);
    }

}


void TorPTClient::parseVariables(Parameters *p) {
    std::string error;

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Starting checking if vars are set\"");
    //No need to parse if mandatory vars are not set
    checkIfVarsAreSet(p);
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Parsing common vars\"");
    //Parse the common vars
    TorPT::parseCommonVars(p);
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Finished Parsing common vars\"");

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTClient Checking -transports\"");
    if(p->transports != ""){
        TorPT::validateListString(p->transports, SUPPORTED_TRANSPORTS,&pt_transports);
            if(pt_transports.size() <= 0){
                display(PT_ARG_ERROR, "Unsupported transports specified");
                exit(1);
        }
    }
    else{
        pt_transports.push_back(p->transport);
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTClient Checking -proxylistenaddr\"");
    
    if(p->proxylistenaddr != ""){
            error = TorPT::validateURI(p->proxylistenaddr, &pt_proxy_listen_addr);

            if(error != ""){
                display(PT_ARG_ERROR, error);
                exit(1);
            }
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTClient Checking -proxylistenhost and -proxylistenport\"");

    if(p->proxylistenhost != "" && p->proxylistenport != 0 ){
            error = TorPT::validateURI(p->proxylistenhost + ":" + std::to_string(p->proxylistenport), &pt_proxy_listen_host, &pt_proxy_listen_port);
            
            if(error != ""){
                display(PT_ARG_ERROR, error);
                exit(1);
            }

            pt_proxy_listen_addr = p->proxylistenhost + ":" + std::to_string(p->proxylistenport);
    }

    /* TODO: Properly check proxy address*/
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTClient Checking -proxy\"");
    pt_upstream_proxy = p->proxy;

}


/**
 * Notifies the Parent Proxy that the SOCKS Proxy is alive and well to start
 * receiving requests OR in case of failure
 */
void TorPTClient::notifyParentAboutProxy(short proxy_status) {

    if (proxy_status == -1) {
        display(PT_C_METHOD_ERROR, TRANSPORT_NAME, "Proxy has crashed!");
        return;
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=PT_CLIENT_TRANSPORTS");

    //For each transport initialized, the PT proxy reports the listener status
    //back to the parent via messages to stdout.
    std::vector<std::string>::const_iterator it = std::find(pt_transports.begin(), pt_transports.end(), TRANSPORT_NAME);

    //Parent process recognize our transport name as a transport
    if (it != pt_transports.end()) {
        std::vector<std::string> Parameters = std::vector<std::string>();
        Parameters.push_back(pt_transports.front());
        Parameters.push_back(SOCKS_VER);
        Parameters.push_back(pt_proxy_listen_addr);
        //display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=CMETHOD set");
        display(PT_C_METHOD, Parameters);

    } else {
        display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"CMETHOD not set\"");
        //there are no pt compatible with the client
        display(PT_C_METHOD_ERROR, TRANSPORT_NAME, "No cloak available");
    }

    display(PT_C_METHOD_DONE);
}



