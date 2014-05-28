// $Id: cixclient.cpp,v 1.2 2014-05-27 23:50:13-07 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "logstream.h"
#include "sockets.h"
#include "cixlib.h"

logstream elog (cerr);
struct cixclient_exit: public exception {};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.cix_command = CIX_LS;
   elog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   elog << "received header " << header << endl;
   if (header.cix_command != CIX_LSOUT) {
      elog << " sent CIX_LS, server did not return CIX_LSOUT" << endl;
   }else {
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes);
      elog << "received " << header.cix_nbytes << " bytes" << endl;
      buffer[header.cix_nbytes] = '\0';
      cout << buffer;
   }
}

void cix_put (client_socket& server) {
   cix_header header;
   header.cix_command = CIX_LS;
   elog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   elog << "received header " << header << endl;
   if (header.cix_command != CIX_LSOUT) {
      elog << " sent CIX_LS, server did not return CIX_LSOUT" << endl;
   }else {
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes);
      elog << "received " << header.cix_nbytes << " bytes" << endl;
      buffer[header.cix_nbytes] = '\0';
      cout << buffer;
   }
}


unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"put" , CIX_PUT },
   {"rm"  , CIX_RM  },
};

int main (int argc, char** argv) {
   elog.set_execname (basename (argv[0]));
   elog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   string host = args.size() < 1 ? "localhost" : args[0];
   in_port_t port = args.size() < 2 ? 50000 : stoi (args[1]);
   elog << to_string (hostinfo()) << endl;
   try {
      elog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      elog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cixclient_exit();
         elog << "command " << line << endl;
         const auto& itor = command_map.find (line);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
         try {
            switch (cmd) {
               case CIX_EXIT:
                  throw cixclient_exit();
                  break;
               case CIX_HELP:
                  cix_help();
                  break;
               case CIX_LS:
                  cix_ls (server);
                  break;
               default:
                  elog << line << ": invalid command" << endl;
                  break;
            }
         }catch (socket_error& error) {
            elog << error.what() << endl;
         }
      }
   }catch (socket_error& error) {
      elog << error.what() << endl;
   }catch (cixclient_exit& error) {
   }
   elog << "finishing" << endl;
   return 0;
}


