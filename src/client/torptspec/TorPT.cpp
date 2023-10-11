#include "TorPT.hpp"
#include "../common/Common.hpp"


void TorPT::display(std::string message) {
    std::cout << message << std::endl;
}

void TorPT::display(std::string prefix, std::vector<std::string> messages) {
    std::ostringstream os;
    os << prefix << " ";
    for (std::string message : messages)
        os << message << " ";
    
    os << std::endl;

    std::cout << os.str();
}

void TorPT::display_version(std::string prefix, std::vector<std::string> messages) {
    std::ostringstream os;
    os << prefix << " ";
    int counter = 0;
    for (std::string message : messages){
        os << message;
        counter++;

        if(messages.size() > counter)
            os << ",";
    }
    
    os << std::endl;

    std::cout << os.str();
}

void TorPT::display(std::string prefix, std::map<std::string, std::string> messages) {
    std::ostringstream os;
    std::map<std::string, std::string>::iterator it;

    os << prefix << " ";

    for (it = messages.begin(); it != messages.end(); ++it) {
        os << it->first << "=" << it->second << " ";
    }
    os << std::endl;

    std::cout << os.str();
}

void TorPT::display(std::string prefix, std::string message) {
    std::cout << prefix << " " << message << std::endl;
}

void TorPT::display(std::string prefix, std::string message, std::string env_var) {
    std::cout << prefix << " " << message << " " << env_var << std::endl;
}

int TorPT::waitUntilStdinClose() {
    fd_set read_set;

    while (true) {
    FD_ZERO (&read_set);
    FD_SET (fileno(stdin), &read_set);

    if (select (FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
            return -1;
        }

        if (FD_ISSET (fileno(stdin), &read_set)) {
            return 0;
        }
    }
}

bool TorPT::validateString(std::string input, const std::vector<std::string> values){
    return std::find(values.begin(), values.end(), input) != values.end();
}

void TorPT::validateListString(std::string inputs, const std::vector<std::string> values, std::vector<std::string> *outputs ){
    std::string current;
    std::stringstream inputs_stream(inputs);

    while (std::getline(inputs_stream, current, ',')){  
         if(validateString(current,values)){
            outputs->push_back(current);
         }
    }   
}


bool TorPT::createStatePath(std::string path){
    return std::filesystem::create_directory(path);
}

std::ifstream TorPT::openFile(std::string filePath){
    std::ifstream file; 

    if(std::filesystem::exists(filePath)){
        file.open(filePath);
    }
    
    return file;
}

bool TorPT::validateAddress(std::string url) {
    std::regex url_regex ("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
    std::smatch url_match_result;

    return std::regex_match(url, url_match_result, url_regex);
    
}

std::string TorPT::validateURI(std::string uri, std::string *ptr_addr, std::string *ptr_port){
    std::vector<std::string> Parameters;

    Parameters = boost::split(Parameters, uri, boost::is_any_of(":"));

    int port;
    std::string addr;

    if(Parameters.size() == 2){
        try{
            addr = Parameters[0];
            port = std::stoi(Parameters[1]);
        }catch(...){
            return "[ERROR]: Incorrectly formatted address provided ";
        }

        if(!validateAddress(addr)){
            return "[ERROR]: Invalid address provided "; 
        }
        if(port < 1024 || port > 65535){
            return "[ERROR]: Invalid port provided ";
        }
        if(ptr_port == NULL){
            ptr_addr = &addr;
        }
        else
        {
            ptr_addr = &addr;
            *ptr_port = std::to_string(port);
        }
    }
    else{
        return "[ERROR]: Incorrectly formatted address provided ";
    }

    return "";
}

bool TorPT::validateProxy(std::string proxy){
    std::regex url_regex ("");
    std::smatch url_match_result;

    return std::regex_match(proxy, url_match_result, url_regex);
}

std::string TorPT::validateBindAddrs(std::string bindaddr, std::vector<std::string> transports){
    std::vector<std::string> Parameters;
    std::vector<std::string> parts;

    std::string transport;
    std::string value;
    std::string address;

    Parameters = boost::split(Parameters, bindaddr, boost::is_any_of(","));

    //Iterate through every bind address for every transport
    for(int i=0; i < Parameters.size(); i++){
        parts = boost::split(parts, Parameters[i], boost::is_any_of("-"));

        if(parts.size() != 2){
            return "[ERROR]: Incorrectly formatted bind address provided ";
        }

        transport = parts[0];
        value = parts[1];

        if(transport != transports[i]){
            return "[ERROR]: Invalid transport name or order provided";
        }

        parts = boost::split(parts, value, boost::is_any_of(":"));

        if(parts.size() != 2){
            return "[ERROR]: Incorrectly formatted bind address provided ";
        }

        address = parts[0];

        if(!validateAddress(address)){
            return "[ERROR]: Invalid bind address provided ";
        }
    }
    return "";
}

std::string TorPT::validateAuthCookie(std::string path){
    if(!std::filesystem::exists(path)){
        return "[ERROR]: Provided AuthCookie path does not exist";
    }
    return "";
}

inline void TorPT::checkIfCommonVarsAreSet(Parameters *p) {
    if(p->transport == "" && p->transports == ""){
        display(PT_ARG_ERROR, ENV_VAR_NSET, PT_TRANSPORT_TRANSPORTS);
        exit(1);
    }

    if(p->transport != "" && p->transports != ""){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_TRANSPORT_TRANSPORTS);
        exit(1);
    }

    if(p->client && p->server ){
        display(PT_ARG_ERROR, ENV_VAR_SET, PT_CLIENT_SERVER);
        exit(1);
    }
}

void TorPT::parseCommonVars(Parameters *p) {
    std::string error;

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"Checking common vars\"");
    //If any of mandatory vars are not set abort immediately
    checkIfCommonVarsAreSet(p);

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -ptversion\"");
    validateListString(p->ptversion, SUPPORTED_PT_VERSION,&pt_versions_supported);

    if(pt_versions_supported.size() <= 0){
        display(PT_VER_ERROR, "Unsupported PT Versions specified");
        exit(1);
    }
    

    display_version(PT_VER_INFO, pt_versions_supported);
    
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -state\"");
    if(p->state != ""){
        pt_state_location = p->state;
        createStatePath(p->state);
    }
    
    pt_exit_on_stdin_close = p->exit_on_stdin_close;

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -ipcLogLevel\"");

    if(!validateString(p->ipcLogLevel, SUPPORTED_LOG_LEVELS)){
        display(PT_ARG_ERROR, "Unsupported IPC Log Level specified");
        exit(1);
    }
    pt_ipc_log_level = p->ipcLogLevel;

    /* Update global ipc log level variable*/
    if (pt_ipc_log_level == "NONE"){
        IPC_LOG_LEVEL = LogLevel::NONE;
    }
    else if(pt_ipc_log_level == "ERROR"){
        IPC_LOG_LEVEL = LogLevel::ERROR;
    }
    else if(pt_ipc_log_level == "WARN"){
        IPC_LOG_LEVEL = LogLevel::WARN;
    }
    else if(pt_ipc_log_level == "INFO"){
        IPC_LOG_LEVEL = LogLevel::INFO;
    }
    else if(pt_ipc_log_level == "DEBUG"){
        IPC_LOG_LEVEL = LogLevel::DEBUG;
    }
    else{
        display(PT_ARG_ERROR, "Unsupported IPC Log Level specified");
        exit(1);
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -transport\"");
    if(p->transport != ""){
        if(!validateString(p->transport, SUPPORTED_TRANSPORTS)){
            display(PT_ARG_ERROR, "Unsupported transport specified");
            exit(1);
        }
        pt_transport = p->transport;
    }
    
    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -optionsFile\"");
    if(p->optionsFile != ""){

        std::ifstream optionsFile = openFile(p->optionsFile);
    
        if(!optionsFile.is_open()){
            display(PT_ARG_ERROR, "Provided options file does not exist");
            exit(1);
        }

        pt_options_file = p->optionsFile;
    }

    display(PT_STATUS, "TRANSPORT=cloak", "DEBUG=\"TorPT Checking -mode\"");
    if(!validateString(p->mode, SUPPORTED_MODES)){
        display(PT_ARG_ERROR, "Unsupported Proxy Mode specified");
        exit(1);
    }
    pt_mode = p->mode;

}



