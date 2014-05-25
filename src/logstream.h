// $Id: logstream.h,v 1.1 2014-05-25 12:44:05-07 - - $

//
// class logstream
// replacement for initial cout so that each call to a logstream
// will prefix the line of output with an identification string
// and a process id.  Template functions must be in header files
// and the others are trivial.
//

#ifndef __LOGSTREAM_H__
#define __LOGSTREAM_H__

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <sys/types.h>
#include <unistd.h>

class logstream {
   private:
      ostream& out;
      string execname;
   public:

      // Constructor may or may not have the execname available.
      logstream (ostream& out, const string& execname = ""):
                 out (out), execname (execname) {
      }

      // First line of main should set_execname if logstream is global.
      void set_execname (const string& name) { execname = name; }

      // First call should be the logstream, not cout.
      // Then forward result to the standard ostream.
      template <typename T>
      ostream& operator<< (const T& obj) {
         assert (execname.size() > 0);
         out << execname << "(" << getpid() << "): " << obj;
         return out;
      }

};

#endif

