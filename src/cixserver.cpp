// $Id: cixserver.cpp,v 1.1 2014-05-25 12:44:05-07 - - $

#include <iostream>
using namespace std;

#include <libgen.h>

#include "cixlib.h"
#include "logstream.h"
#include "sockets.h"

logstream elog (cerr);

void reply_ls (accepted_socket& client_sock, cix_header& header) {
   FILE* ls_pipe = popen ("ls -l", "r");
   if (ls_pipe == NULL) throw socket_sys_error ("popen(\"ls -l\")");
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   header.cix_command = CIX_LSOUT;
   header.cix_nbytes = ls_output.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   elog << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   elog << "sent " << ls_output.size() << " bytes" << endl;
}


int main (int argc, char**argv) {
   elog.set_execname (basename (argv[0]));
   vector<string> args (&argv[1], &argv[argc]);
   int client_fd = stoi (args[0]);
   elog << "starting client_fd " << client_fd << endl;
   try {
      accepted_socket client_sock (client_fd);
      elog << "connected to " << to_string (client_sock) << endl;
      for (;;) {
         cix_header header;
         recv_packet (client_sock, &header, sizeof header);
         elog << "received header " << header << endl;
         switch (header.cix_command) {
            case CIX_LS:
               reply_ls (client_sock, header);
               break;
            default:
               elog << "invalid header from client" << endl;
               elog << "cix_nbytes = " << header.cix_nbytes << endl;
               elog << "cix_command = " << header.cix_command << endl;
               elog << "cix_filename = " << header.cix_filename << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      elog << error.what() << endl;
   }
   elog << "finishing" << endl;
   return 0;
}

