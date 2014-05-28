// $Id: cixclient.cpp,v 1.5 2014-05-28 10:33:19-07 - - $

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cerrno>
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

vector<string> split (const string& line, const string& delimiters) {
   vector<string> words;
   int end = 0;
   for (;;) {
      size_t start = line.find_first_not_of (delimiters, end);
      if (start == string::npos) break;
      end = line.find_first_of (delimiters, start);
      words.push_back (line.substr (start, end - start));
   }
   return words;
}

void cix_put (client_socket& server, vector<string>& params) {
   //if (params.size() != 2) 
   //elog << params << "missing filenames" << endl;
   cix_header header;
   ifstream file(params[1], ios::in|ios::binary|ios::ate); 
   if (!file.is_open()) {
      elog << params[1] << ": " << strerror(errno) << endl;
   } else{
      streampos size {file.tellg()};
      char *memblock = new char [size];
      file.seekg (0, ios::beg);
      file.read (memblock, size);
      file.close();
      header.cix_command = CIX_PUT;
      header.cix_nbytes = size;
      strcpy(header.cix_filename, params[1].c_str());
      elog << "sending header " << header << endl;
      send_packet (server, &header, sizeof header);
      send_packet (server, memblock, size);
      recv_packet (server, &header, sizeof header);
      elog << "received header " << header << endl;
      if (header.cix_command != CIS_ACK)
         elog << strerror(header.cix_nbytes) << endl;
      else
         elog << "put " << params[1] << " successed" << endl;
   }
}

void cix_get (client_socket& server, vector<string>& params) {
   //if (params.size() != 2)
   //elog << params << "missing filenames" << endl;
   cix_header header;
   string filename = params[1];
   header.cix_command = CIX_GET;
   strcpy (header.cix_filename , filename.c_str());
   elog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   elog << "received header " << header << endl;
   if (header.cix_command != CIX_FILE) {
      elog << strerror(header.cix_nbytes) << endl;
   }else {
      string filename {header.cix_filename};
      //char *fileout = header.cix_filename;
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes); //payload
      elog << "received " << header.cix_nbytes << " bytes" << endl;
      filename.append(".got");
      //ofstream fileout (filename.c_str(), ios::out || ios::binary);
      ofstream fileout;
      fileout.open ("crap", ios::out | ios::binary);
      if (!fileout.is_open()) {
         elog << "can't open: " << filename
              << " " << strerror(errno) << endl;
      } else{
         fileout.write (buffer, header.cix_nbytes);
         fileout.close();
      }
   }
}

void cix_rm (client_socket& server, vector<string>& params) {
   cix_header header;
   string filename = params[1];
   header.cix_command = CIX_RM;
   strcpy (header.cix_filename , filename.c_str());
   elog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   elog << "received header " << header << endl;
   if (header.cix_command == CIS_NAK) {
      elog << strerror(header.cix_nbytes) << endl;
   }
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


unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"put" , CIX_PUT },
   {"get" , CIX_GET },
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
         vector<string> params = split (line, " \t");
         elog << "params0 " << params[0] << endl;
         if(params.size() > 1) elog << "params1 " << params[1] << endl;

         const auto& itor = command_map.find (params[0]);
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
               case CIX_RM:
                  cix_rm (server, params);
                  break;
               case CIX_LS:
                  cix_ls (server);
                  break;
               case CIX_GET:
                  cix_get (server, params);
                  break;
               case CIX_PUT:
                  cix_put (server, params);
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


