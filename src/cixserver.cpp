// $Id: cixserver.cpp,v 1.5 2014-05-28 10:31:25-07 - - $

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cerrno>
using namespace std;

#include <libgen.h>

#include "cixlib.h"
#include "logstream.h"
#include "sockets.h"

logstream elog (cerr);

void reply_rm (accepted_socket& client_sock, cix_header& header) {
   if (unlink (header.cix_filename)) {
      elog << header.cix_filename << ": " << strerror(errno) << endl; 
      header.cix_nbytes = errno;
      header.cix_command = CIS_NAK;
      elog << "sending NAK header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
   } else{
      header.cix_nbytes = 0;
      header.cix_command = CIS_ACK;
      elog << "sending ACK header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
   }
}

void reply_put (accepted_socket& client_sock, cix_header& header) {
   string filename {header.cix_filename};
   uint32_t size {header.cix_nbytes};
   char buffer[size + 1];
   filename.append(".gotput");
   recv_packet (client_sock, buffer, size);
   ofstream fileout;
   fileout.open (filename, ios::out | ios::binary);
   if (!fileout.is_open()){
      elog << "can't open: " << filename
         << " " << strerror(errno) << endl;
   } else {
      fileout.write (buffer, size);
      fileout.close();
      header.cix_nbytes = 0;
      header.cix_command = CIS_ACK;
      elog << "sending ACK header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
   }
}

void reply_get (accepted_socket& client_sock, cix_header& header) {
   ifstream file(header.cix_filename, ios::in|ios::binary|ios::ate); 
   if (!file.is_open()) {
      elog << header.cix_filename << ": " << strerror(errno) << endl; 
      header.cix_nbytes = errno;
      header.cix_command = CIS_NAK;
      elog << "sending NAK header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
   } else{ 
      streampos size {file.tellg()};
      char *memblock = new char [size];
      file.seekg (0, ios::beg);
      file.read (memblock, size);
      file.close();
      header.cix_command = CIX_FILE;
      header.cix_nbytes = size;
      elog << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
      send_packet (client_sock, memblock, size);
      elog << "sent " << size << " bytes" << endl;
      delete[] memblock;
   }
}


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
   if(argc!=2) {
      elog << "must be forked from a daemon %d" << argc << endl;
      return 0;
   }

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
            case CIX_GET:
               reply_get (client_sock, header);
               break;
            case CIX_PUT:
               reply_put (client_sock, header);
               break;
            case CIX_RM:
               reply_rm (client_sock, header);
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

