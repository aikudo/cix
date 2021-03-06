// $Id: cixdaemon.cpp,v 1.2 2014-05-27 23:50:13-07 - - $

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "logstream.h"
#include "sockets.h"

logstream elog (cerr); //create an obj elog using cerr as output

void fork_cixserver (server_socket& server, accepted_socket& accept) {
   pid_t pid = fork();
   if (pid == 0) { // child
      server.close();
      execlp ("cixserver", "cixserver-forked",
              accept.to_string_socket_fd().c_str(), nullptr);
      // Can't get here?!
      elog << "execlp failed: " << strerror (errno) << endl;
      exit (1);
   }else {
      accept.close();
      if (pid < 0) {
         elog << "fork failed: " << strerror (errno) << endl;
      }else {
         elog << "forked cixserver pid " << pid << endl;
      }
   }
}

void reap_zombies() {
   for (;;) {
      int status;
      pid_t child = waitpid (-1, &status, WNOHANG);
      if (child <= 0) break;
      elog << "child " << child
           << " exit " << (status >> 8)
           << " signal " << (status & 0x7F)
           << " core " << (status >> 7 & 1) << endl;
   }
}


int main (int argc, char** argv) {
   elog.set_execname (basename (argv[0]));
   vector<string> args (&argv[1], &argv[argc]);
   in_port_t port = args.size() < 1 ? 50000 : stoi (args[0]);
   try {
      server_socket listener (port);
      for (;;) {
         elog << to_string (hostinfo()) << " accepting port "
              << to_string (port) << endl;
         accepted_socket client_sock;
         //this a blocking
         listener.accept (client_sock); //waiting for a client?
         elog << "accepted " << to_string (client_sock) << endl;
         try {
            fork_cixserver (listener, client_sock);
            reap_zombies();
         }catch (socket_error& error) {
            elog << error.what() << endl;
         }
      }
   }catch (socket_error& error) {
      elog << error.what() << endl;
   }
   return 0;
}

