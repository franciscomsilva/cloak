#ifndef TORPTSERVER_H
#define TORPTSERVER_H

#include "TorPT.hpp"
#include "../common/Common.hpp"

//Tor Vars
#define PT_SERVER_TRANSPORT_BINDHOST_BINDPORT_BINDADDR    "-bindaddr || -bindhost && -bindport"
#define PT_SERVER_TRANSPORT_BINDHOST_BINDPORT  "-bindhost -bindport"
#define PT_TARGET_TARGETHOST_TARGETPORT        "-target || -targethost && -targerport"
#define PT_TARGETHOST_TARGETPORT        " -targethost -targerport"
#define PT_EXTORPORT_AUTHCOOKIE        " -extorport -authcookie"
#define PT_SERVER_TRANSPORTS_OPT "-options"
#define PT_SERVER_BINDADDR       "-bindaddr"
#define PT_EXT_SERVER_PORT       "-extorport"
#define PT_AUTH_COOKIE           "-authcookie"



class TorPTServer : public TorPT {

  public:

    TorPTServer() {
        // pt_bindaddr = std::map<std::string, std::string>();
    }

    void initialize(Parameters *p, int run_mode);

    std::vector<std::string>           getTransports()        {return pt_transports;}

    std::map<std::string, std::string> getTransportsOptions() {return pt_transport_options;}

    // std::map<std::string, std::string> getBindAddresses()     {return pt_bindaddr;}

    std::string                        getExtendedOnionPort() {return pt_extor_port;}

    std::string                        getAuthCookie()        {return pt_auth_cookie;}

    std::string                        getBindAddressPT()     {return pt_bind_addr;}

    int                                getTargetPort()       {return std::stoi(pt_target_port);}

    int                                getBindPort()         {return std::stoi(pt_bind_port);}



protected:

    inline void checkIfVarsAreSet(Parameters *p);

    void parseVariables(Parameters *p);

    void notifyParentAboutProxy(short proxy_status);

private:
  
    /*
      -transports
    
      Specifies the PT protocols the server proxy should initialize, as a comma separated list of PT names.

      PTs SHOULD ignore PT names that it does not recognize.
    */

    std::vector<std::string> pt_transports;

    /*
      -options

      Specifies per-PT protocol configuration directives, as a JSON string value with options that are to be passed to
      the transport.

      If there are no arguments that need to be passed to any of PT transport protocols, -options MAY be omitted.

      If a PT Client requires a server address, then this can be communicated by way of the transport options. 
      For consistency, transports SHOULD name this option "serverAddress" and it SHOULD have a format of <address>:<port>.
      Unless otherwise specified in the documentation of the specific transport being used, the address can be an IPv4 IP address,
      an IPv6 IP address, or a domain name. Not all transports require a server address and some will require multiple server 
      addresses, so this convention only applies to the case where the transport requires a single server address.
    */

    std::map<std::string, std::string> pt_transport_options;

    /*
      -bindaddr

      A comma separated list of <key>-<value> pairs, where <key> is a PT name and <value> is the <address>:<port> on 
      it should listen for incoming client connections.

      The keys holding transport names MUST be in the same order as they appear in -transports.

      The <address> MAY be a locally scoped address as long as port forwarding is done externally.

      The <address>:<port> combination MUST be an IP address supported by bind(), and MUST NOT be a host name.
    
    */

    std::string pt_bind_addr;

    /*
      -bindhost

      Specifies the <address> part of the server bind address when used in conjunction with -transport and -bindport.

      The <address> MAY be a locally scoped address as long as port forwarding is done externally.

      The <address> MUST be an IP address supported by bind(), and MUST NOT be a host name.

      Applications MUST NOT set more than one <address> using -bindhost.

      The bind address MUST be set, either using a combined -bindaddr flag or in separate parts using -transport, 
      -bindhost, and -bindport.
    */

    std::string pt_bind_host = "127.0.0.1";

    /*
      -bindport
      
      Specifies the <port> part of the server bind address when used in conjunction with -transport and -bindhost.

      Applications MUST NOT set more than one <port> using -bindport.

      The bind port MUST be set, either using a combined -bindaddr flag or in separate parts using -transport, -bindhost, and -bindport.  
    */

    std::string pt_bind_port = "1080";

  

  /*
    -target

    Specifies the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate,
    as an <address>:<port>. Unless otherwise specified in the documentation of the specific transport being used, the 
    address can be an IPv4 IP address, an IPv6 IP address, or a domain name.

    Connections to the target destination MUST only contain application payload. If the parent process requires the actual
    source IP address of client connections (or other metadata), it should set -extorport instead.

    The target destination MUST be set. A combined address and port can be set using -target. Alternatively, -targethost and -targetport
    can be used to separately set the address and port respectively. If a combined address and port is specified then a separate host
    and port cannot be specified and vice versa.
    */

  std::string pt_target = "127.0.0.1:9090";

  /*
    -targethost

    Specifies the <address> of the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate.
    Unless otherwise specified in the documentation of the specific transport being used, the address can be an IPv4 IP address, an IPv6 IP address, or a domain name.

    Connections to the target destination MUST only contain application payload. If the parent process requires the actual source IP address of client connections
    (or other metadata), it should set -extorport instead.

    The target destination MUST be set. A combined address and port can be set using -target.
    Alternatively, -targethost and -targetport can be used to separately set the address and port respectively. If a combined address and port is specified then a separate
    host and port cannot be specified and vice versa.
   */

  std::string pt_target_host = "127.0.0.1";

  /*
    -targetport
    Specifies the <port> of the destination that the PT reverse proxy should forward traffic to after transforming it as appropriate.

    Connections to the target destination MUST only contain application payload. If the parent process requires the actual source IP address
    of client connections (or other metadata), it should set -extorport instead.

    The target destination MUST be set. A combined address and port can be set using -target. Alternatively, -targethost and -targetport can be used to
    separately set the address and port respectively. If a combined address and port is specified then a separate host and port cannot be specified and vice versa.

    If -targetport is specified, then -targethost must also be specified
   */

  std::string pt_target_port = "9090";

  /*
    -extorport

    Specifies the destination that the PT reverse proxy should forward traffic to, via the Extended ORPort protocol [EXTORPORT] 
    as an <address>:<port>.

    The Extended ORPort protocol allows the PT reverse proxy to communicate per-connection metadata such as the PT name 
    and client IP address/port to the parent process.
  */

  std::string pt_extor_port = "127.0.0.1:9090";

  /*
    -authcookie
    Specifies an absolute filesystem path to the Extended ORPort authentication cookie, required to communicate with the
    Extended ORPort specified via -extorport .

    If the parent process is not using the ExtORPort protocol for incoming traffic, -authcookie MUST be omitted.
  */

  std::string pt_auth_cookie;

};


#endif // TORPTSERVER_H