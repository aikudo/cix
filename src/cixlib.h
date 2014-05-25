// $Id: cixlib.h,v 1.1 2014-05-25 12:44:05-07 - - $

#ifndef __CIXLIB__H__
#define __CIXLIB__H__

#include <cstdint>
#include <cstring>
#include <iostream>
using namespace std;

#include "sockets.h"

enum cix_command {CIX_ERROR = 0, CIX_EXIT,
                  CIX_GET, CIX_HELP, CIX_LS, CIX_PUT, CIX_RM,
                  CIX_FILE, CIX_LSOUT, CIS_ACK, CIS_NAK};

size_t constexpr CIX_FILENAME_SIZE = 59;
struct cix_header {
   uint32_t cix_nbytes {0};
   uint8_t cix_command {0};
   char cix_filename[CIX_FILENAME_SIZE];
   cix_header() { memset (cix_filename, 0, CIX_FILENAME_SIZE); }
};

void send_packet (base_socket& socket,
                  const void* buffer, size_t bufsize);

void recv_packet (base_socket& socket, void* buffer, size_t bufsize);

ostream& operator<< (ostream& out, const cix_header& header);

#endif

