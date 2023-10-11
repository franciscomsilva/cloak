#ifndef TORPT_H
#define TORPT_H

#include <bits/stdc++.h> 
#include <iostream> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <algorithm>
#include <regex>
#include <boost/asio.hpp>
#include <filesystem>


#define TRANSPORT_NAME "cloak"
#define SOCKS_VER      "socks5"
#define CLI_SOCKS_ADDR_PORT "127.0.0.1:1080"
#define SERVER_LISTEN4CLIREQS_PORT "127.0.0.1:1089"
#define SER_ARGS             ""



/* The size of temporary buffer for forwarding data */
#define PT_BUF_SIZE 4096


//Pluggable Transports Output Standards
const std::string PT_ARG_ERROR = "ARG-ERROR";
const std::string PT_VER_ERROR = "VERSION-ERROR";
const std::string PT_VER_INFO  = "VERSION";

const std::string PT_PRX_DONE   = "PROXY DONE";
const std::string PT_PRX_ERROR  = "PROXY-ERROR";

const std::string PT_C_METHOD         = "CMETHOD";
const std::string PT_C_METHOD_ERROR   = "CMETHOD-ERROR";
const std::string PT_C_METHOD_DONE    = "CMETHODS DONE";

const std::string PT_S_METHOD         = "SMETHOD";
const std::string PT_S_METHOD_ERROR   = "SMETHOD-ERROR";
const std::string PT_S_METHOD_DONE    = "SMETHODS DONE";

const std::string PT_STATUS        = "STATUS";
const std::string LOG              = "LOG";
const std::string LOG_SEVERITY     = "SEVERITY";
const std::string LOG_MESSAGE      = "MESSAGE";


#define TORVARS_VERSION "1"

//Tor Vars
#define PT_TRANSPORT_TRANSPORTS "-transport -transports"
#define PT_CLIENT_SERVER "-client -server"



//Tor ENV_ERR Descriptions
const std::string ENV_VAR_NSET = "The following argument is not set:";
const std::string ENV_VAR_SET  = "The following argument is set when it shouldn't:";

/* Struct to save command line arguments values*/

struct Parameters
{
    /* Common configuration parameter variables */
    std::string ptversion;
    std::string state;
    bool exit_on_stdin_close;
    std::string ipcLogLevel;
    std::string transport;
    std::string optionsFile;
    std::string transports;
    std::string mode;

    /* Client configuration parameter variables */
    bool client;
    std::string proxylistenaddr;
    std::string proxylistenhost;
    int proxylistenport;
    std::string proxy;

    /* Server configuration parameter variables */
    bool server;
    std::string options;
    std::string bindaddr;
    std::string bindhost;
    int bindport;
    std::string target;
    std::string targethost;
    int targetport;
    std::string extorport;
    std::string authcookie;

};


class TorPT {

    public:
        bool exitOnStdinClose() {
            return pt_exit_on_stdin_close;
        }

        int waitUntilStdinClose();

        
    protected:

      

        /* Supported values */
        const std::vector<std::string> SUPPORTED_PT_VERSION {"1","1.0","3.0"};
        const std::vector<std::string> SUPPORTED_TRANSPORTS {"cloak"};
        const std::vector<std::string> SUPPORTED_LOG_LEVELS {"NONE","ERROR","WARN","INFO","DEBUG"};
        const std::vector<std::string> SUPPORTED_MODES {"transparent-TCP","transparent-UDP","socks5","STUN"};
        
        inline void checkIfCommonVarsAreSet(Parameters *p);

        void parseCommonVars(Parameters *p);

        void display(std::string message);

        void display(std::string prefix, std::string message);

        void display(std::string prefix, std::vector<std::string> messages);

        void display(std::string prefix, std::string message,
                     std::string env_var);

        void display(std::string prefix, std::map<std::string,
                     std::string> messages);

        void display_version(std::string prefix, std::vector<std::string> messages);

        void validateListString(std::string inputs, const std::vector<std::string> values, std::vector<std::string> *outputs );

        bool createStatePath(std::string path);

        bool validateString(std::string input, const std::vector<std::string> values);

        std::ifstream openFile(std::string filePath);

        bool validateAddress(std::string url);

        std::string validateURI(std::string uri, std::string *ptr_addr, std::string *ptr_port = NULL);

        bool validateProxy(std::string proxy);

        std::string validateBindAddrs(std::string bindaddr, std::vector<std::string> transports);

        std::string validateAuthCookie(std::string path);


    private:

    /*
        -state

        Specifies a path to a directory where the PT is allowed to store state that will be persisted across invocations.
        This can be either an absolute path or a relative path. If a relative path is used, it is assumed to be relative
        to the current directory. The directory is not required to exist when the PT is launched, however PT implementations 
        SHOULD be able to create it as required.
    */
    
    std::string pt_state_location;

    /*
        -ptversion

        Specifies the versions of the Pluggable Transport specification the parent process supports, 
        delimited by commas. All PTs MUST accept any well-formed list, as long as a compatible version is present.
        Valid versions MUST consist entirely of non-whitespace, non-comma printable ASCII characters.
        The version of the Pluggable Transport specification as of this document is "3.0".
    */
    std::vector<std::string> pt_versions_supported;

    /*
        -exit-on-stdin-close

        Specifies that the parent process will close the PT proxy's standard input (stdin) stream to indicate that
        the PT proxy should gracefully exit.
        PTs MUST NOT treat a closed stdin as a signal to terminate unless this flag is present and is set to "1".
        PTs SHOULD treat stdin being closed as a signal to gracefully terminate if this flag is set to "1".
    */
    bool pt_exit_on_stdin_close;

    /*
       -ipcLogLevel

        Controls what log messages are sent from the dispatcher to the application using LOG messages.
    */
    std::string pt_ipc_log_level;

    /*
        -transport

        Specifies the name of the PT to use.

        The application MUST set either a single transport with -transport or a list of transports with -transports.
        The application MUST NOT set both a single transport and a list of transports simultaneously.
    */
    std::string pt_transport;

    /*
        -optionsFile

        Specifies the path to a file containing the transport options. This path can be either an absolute path or a relative path.
        If a relative path is used, it is relative to the current directory.
        The contents of the file MUST be in the same format as the argument to the -options parameter.
        The application MUST NOT specify both -options and -optionsFile simultaneously.
    */

    std::string pt_options_file;

     /*
        -mode

        The default if no mode is specified is socks5.
    */

    std::string pt_mode;

    /*
        -client

        Specifies that the PT proxy should run in client mode.

        If neither -client or -server is specified, the PT proxy MUST launch in client mode.

    */

    bool pt_client;


    /*
        -server

        Specifies that the PT proxy should run in server mode.

        If neither -client or -server is specified, the PT proxy MUST launch in client mode.

    */

    bool pt_server;

};  


#endif // TORPT_H