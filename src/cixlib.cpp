// $Id: cixlib.cpp,v 1.2 2014-05-30 23:42:23-07 - - $

#include <unordered_map>
#include <string>
using namespace std;

#include "cixlib.h"

unordered_map<int,string> cix_command_map {
   {int (CIX_ERROR), "CIX_ERROR"},
   {int (CIX_EXIT ), "CIX_EXIT" },
   {int (CIX_GET  ), "CIX_GET"  },
   {int (CIX_HELP ), "CIX_HELP" },
   {int (CIX_LS   ), "CIX_LS"   },
   {int (CIX_PUT  ), "CIX_PUT"  },
   {int (CIX_RM   ), "CIX_RM"   },
   {int (CIX_FILE ), "CIX_FILE" },
   {int (CIX_LSOUT), "CIX_LSOUT"},
   {int (CIS_ACK  ), "CIS_ACK"  },
   {int (CIS_NAK  ), "CIS_NAK"  },
};


void send_packet (base_socket& socket,
                  const void* buffer, size_t bufsize) {
   const char* bufptr = (const char*) buffer;
   size_t ntosend = bufsize;
   do {
      size_t nbytes = socket.send (bufptr, ntosend);
      bufptr += nbytes;
      ntosend -= nbytes;
   }while (ntosend > 0);
}

void recv_packet (base_socket& socket, void* buffer, size_t bufsize) {
   char* bufptr = (char*) buffer;
   size_t ntorecv = bufsize;
   do {
      size_t nbytes = socket.recv (bufptr, ntorecv);
      if (nbytes == 0) throw socket_error ("socket.recv is closed");
      bufptr += nbytes;
      ntorecv -= nbytes;
   }while (ntorecv > 0);
}

ostream& operator<< (ostream& out, const cix_header& header) {
   const auto& itor = cix_command_map.find (header.cix_command);
   string code = itor == cix_command_map.end() ? "?" : itor->second;
   cout << "{" << header.cix_nbytes << "," << code << "="
        << int (header.cix_command) << ",\"" << header.cix_filename
        << "\"}";
   return out;
}    


string get_cix_server_host (const vector<string>& args, size_t index) {
   if (index < args.size()) return args[index];
   char* host = getenv ("CIX_SERVER_HOST");
   if (host != nullptr) return host;
   return "localhost";
}


in_port_t get_cix_server_port (const vector<string>& args,
      size_t index) {
   string port = "-1";
   if (index < args.size()) port = args[index];
   else {
      char* envport = getenv ("CIX_SERVER_PORT");
      if (envport != nullptr) port = envport;
   }
   return stoi (port);
}


