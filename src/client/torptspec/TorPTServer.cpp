#include "TorPTServer.hpp"


void TorPTServer::initialize(Parameters *p,int run_mode)
{
    assert(run_mode == RUN_FOREGROUND);

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Parsing vars...\"");
    parseVariables(p);
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Starting Proxy...\"");
    notifyParentAboutProxy(0);
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Proxy is running...\"");
}


void TorPTServer::checkIfVarsAreSet(Parameters *p)
{
    if(p->bindaddr != "" && (p->transport != "" || p->bindhost != "" || p->bindport != 0)){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_SERVER_TRANSPORT_BINDHOST_BINDPORT_BINDADDR);
        exit(1);
    }

    if(p->bindaddr == "" && (p->bindhost == "" || p->bindport == 0)){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_SERVER_TRANSPORT_BINDHOST_BINDPORT_BINDADDR);
        exit(1);
    }


    if(p->bindaddr != "" && (p->bindhost != "" || p->bindport != 0)){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_SERVER_TRANSPORT_BINDHOST_BINDPORT_BINDADDR);
        exit(1);
    }


    if(p->bindhost != "" && (p->transport == "" || p->bindport == 0)){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_SERVER_TRANSPORT_BINDHOST_BINDPORT);
        exit(1);
    }

    if(p->bindport != 0 &&  p->bindhost == ""){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_SERVER_TRANSPORT_BINDHOST_BINDPORT);
        exit(1);
    }

    if(p->target == "" && (p->targethost == "" || p->targetport == 0)){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_TARGET_TARGETHOST_TARGETPORT);
        exit(1);
    }

    if(p->target != "" && (p->targethost != "" || p->targetport != 0)){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_TARGET_TARGETHOST_TARGETPORT);
        exit(1);   
    }

    if((p->targethost != "" && p->targetport == 0) || (p->targethost == "" && p->targetport != 0)){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_TARGETHOST_TARGETPORT);
        exit(1);   
    }

    if(p->authcookie != "" && p->extorport == ""){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_EXTORPORT_AUTHCOOKIE);
        exit(1);
    }
}


void TorPTServer::parseVariables(Parameters *p)
{
    //No need to parse if mandatory vars are not set
    checkIfVarsAreSet(p);

    //Parse the common vars
    TorPT::parseCommonVars(p);

    std::string error;


    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -transports\"");
    if(p->transports != ""){
        this->pt_transports = boost::split(this->pt_transports, p->transports, boost::is_any_of(","));

    }
    
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -options\"");
    if(p->options != ""){
        std::vector<std::string> Parameters;
        Parameters = boost::split(Parameters, p->options, boost::is_any_of(";"));
        for (std::string param : Parameters) {
            std::vector<std::string> result;
            result = boost::split(result, param, boost::is_any_of(":"));
            pt_transport_options.insert(std::pair<std::string, std::string>(result[0], result[1]));
        }
    }
    
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -bindaddr\"");

    if(p->bindaddr != ""){
            error = validateURI(p->bindaddr, &pt_bind_host, &pt_bind_port);

            if(error != ""){
                display(PT_ARG_ERROR, error);
                exit(1);
            }

    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -bindhost and -bindport\"");


    if(p->bindhost != ""){
        error = validateURI(p->bindhost + ":" + std::to_string(p->bindport),&pt_bind_host, &pt_bind_port );
        
        if(error != ""){
            display(PT_ARG_ERROR, error);
            exit(1);
        }
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -target\"");

    if(p->target != ""){
            error = validateURI(p->target, &pt_target,&pt_target_port);


            if(error != ""){
                display(PT_ARG_ERROR, error);
                exit(1);
            }

    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -targethost and -targetport\"");

    if(p->targethost != ""){
            error = validateURI(p->targethost + ":" +  std::to_string(p->targetport),&pt_target, &pt_target_port);

            if(error != ""){
                display(PT_ARG_ERROR, error);
                exit(1);
            }

    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -extorport\"");



    if(p->extorport != ""){
        pt_extor_port = p->extorport;
    }

    //split address and port information
    std::vector<std::string> Parameters;
    Parameters = boost::split(Parameters, pt_extor_port, boost::is_any_of(":"));
    std::string pt_extended_addr = Parameters[0];
    std::string pt_extented_port = Parameters[1];
    display(PT_STATUS, "TRANSPORT=cloak", "EXTONIONADDR=" + pt_extended_addr + " EXTONIONPORT=" + pt_extented_port);

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPTServer Checking -authcookie\"");
    
    if(p->authcookie != ""){
        error = validateAuthCookie(p->authcookie);

        if(error != ""){
            display(PT_ARG_ERROR, error);
            exit(1);
        }

        pt_auth_cookie = p->authcookie;
    }
  
    
}

void TorPTServer::notifyParentAboutProxy(short proxy_status) {

    if (proxy_status == -1) {
        display(PT_C_METHOD_ERROR, TRANSPORT_NAME, "Proxy has crashed!");
        return;
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=PT_SERVER_TRANSPORTS");

    //For each transport initialized, the PT proxy reports the listener status
    //back to the parent via messages to stdout.
    std::vector<std::string>::const_iterator it = std::find(pt_transports.begin(), pt_transports.end(), TRANSPORT_NAME);

    //Parent process recognize our transport name as a transport
    if (it != pt_transports.end()) {
        std::vector<std::string> Parameters = std::vector<std::string>();
        Parameters.push_back(pt_transports.front());
        Parameters.push_back(pt_bind_host + ":" + pt_bind_port);
        //display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=CMETHOD set");
        display(PT_S_METHOD, Parameters);

    } else {
        display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"SMETHOD not set\"");
        //there are no pt compatible with the client
        display(PT_S_METHOD_ERROR, TRANSPORT_NAME, "No cloak available");
    }

    display(PT_S_METHOD_DONE);
}