#ifndef __AUXLIB_H__
#define __AUXLIB_H__

#include <stdarg.h>
#include <stdbool.h>

//
// Error message and exit status utilities. 
//

void set_execname (char *argv0);
   //
   // Sets the program name for use by auxlib messages
   //

char *get_execname (void);
   //
   // Return read-only value stored by set_execname.
   //

void eprint_status (char *command, int status);
   //
   // Print the status returned by wait(2) from a subprocess.
   //

void set_exitstatus (int);
   //
   // Sets exit status.
   //

int get_exitstatus (void);
   //
   // Returns exit status. Default is EXIT_SUCCESS.
   //

void veprintf (char *format, va_list args);
   //
   // Prints message to stderr using the vector form of args.
   //

void eprintf (char *format, ...);
   //
   // Print message to stderr according to printf format specified.
   // Usually called for debug output. Precede the message by the 
   // exec name if the format begins with `%:'.
   //

void errprintf (char *format, ...);
   //
   // Print error message according to printf format specified,
   // using eprintf. Set exit status to EXIT_FAILURE.
   //

void syserrprintf (char *object);
   //
   // Print a message resulting from a bad system call. The object
   // in the name of the object causing the problem and the reason
   // is taken from the external variable errno.
   //


//
// Stub message utility.
// 

#define STUBPRINTF(...) \
        __stubprintf (__FILE__, __LINE__, __func__, __VA_ARGS__)
void __stubprintf (char *file, int line, const char *func,
                   char *format, ...);

//
// Debugging utility.
//

void set_debugflags (char *flags);
   //
   // Sets a string of debug flags to be used by DEBUGF statements.
   // Uses the address of the string, and does not copy it, so it
   // must not be dangling. If a particular debug flag has been set,
   // messages are printed. The flag "@" turns on all flags.
   //

bool is_debugflag (char flag);
   //
   // Checks to see if a debugflag has been set.
   // 

#ifdef NDEBUG
#define DEBUGF(FLAG,...)   
#define DEBUGSTMT(FLAG,STMTS) 
#else
void __debugprintf (char flag, char *file, int line, const char *func,
                    char *format, ...);
#define DEBUGF(FLAG,...) \
        __debugprintf (FLAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUGSTMT(FLAG,STMTS) \
        if (is_debugflag (FLAG)) { DEBUGF (FLAG, "\n"); STMTS }

#endif
#endif
