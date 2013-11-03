
#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "auxlib.h"
#include "cmdtree.h"
#include "lyutils.h"

#define MAXLEN 64

typedef struct cmd_attributes *cmdattr_ref;

void cmd_select (cmdtree, cmdattr_ref);

struct cmd_attributes {
   char *lprog;
   char **largs;
   char *rprog;
   char **rargs;
   char *input_r;
   char *output_r;
};

cmdattr_ref new_cmdattr (void) {
   cmdattr_ref new = malloc (sizeof (struct cmd_attributes));
   assert (new != NULL);
   new->lprog = NULL;
   new->largs = calloc (MAXLEN, sizeof (char*));
   assert (new->largs != NULL);
   new->rprog = NULL;
   new->rargs = calloc (MAXLEN, sizeof (char*));
   assert (new->rargs != NULL);
   new->input_r = NULL;
   new->output_r = NULL;
   return new;
}

void free_cmdattr (cmdattr_ref cmdattr) {
   free (cmdattr->largs);
   free (cmdattr->rargs);
   free (cmdattr);
}

void select_children (cmdtree tree, cmdattr_ref cmdattr) {
   cmdtree child = tree->first;
   for (;;) {
      if (child == NULL) break;
      cmd_select (child, cmdattr);
      child = child->next;
   }
}

void cmd_reout (cmdtree tree, cmdattr_ref cmdattr) {
   cmdattr->output_r = tree->first->lexinfo;
}

void cmd_rein (cmdtree tree, cmdattr_ref cmdattr) {
   cmdattr->input_r = tree->first->lexinfo;
}

void cmd_args (cmdtree tree, cmdattr_ref cmdattr) {
   cmdtree child = tree->first;
   int ind = 1;
   while (ind < MAXLEN) {
      if (child == NULL) break; 
      cmdattr->largs[ind++] = child->lexinfo;
      child = child->next;
   }
   cmdattr->largs[ind] = NULL;
}

void cmd_prog (cmdtree tree, cmdattr_ref cmdattr) {
   cmdattr->lprog = tree->first->lexinfo;
   cmdattr->largs[0] = tree->first->lexinfo;
}

void cmd_invoc (cmdtree tree, cmdattr_ref cmdattr) {
   select_children (tree, cmdattr);
}

void cmd_right (cmdtree tree, cmdattr_ref cmdattr) {
   DEBUGF ('t', "tree=%p; attr=%p\n", tree, cmdattr);
}

void cmd_left (cmdtree tree, cmdattr_ref cmdattr) {
   DEBUGF ('t', "tree=%p; attr=%p\n", tree, cmdattr);
}

void cmd_pipe(cmdtree tree, cmdattr_ref cmdattr) {
   select_children (tree, cmdattr);
}

void set_output_redirect (cmdattr_ref cmdattr) {
   int out_fd = -1;
   out_fd = open (cmdattr->output_r, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
   if (out_fd < 0) {
      syserrprintf ("cmdattr->output_r");
      exit (get_exitstatus ());
   }
   if (close (STDOUT_FILENO) < 0) {
      close (out_fd);
      syserrprintf ("close()");
      exit (get_exitstatus ());
   }
   if (dup (out_fd) != STDOUT_FILENO) {
      close (out_fd);
      syserrprintf ("dup()");
      exit (get_exitstatus ());
   }
   close (out_fd);
}

void set_input_redirect (cmdattr_ref cmdattr) {
   int in_fd = -1;
   in_fd = open (cmdattr->input_r, O_RDONLY);
   if (in_fd < 0) {
      syserrprintf (cmdattr->input_r);
      exit (get_exitstatus ());
   }
   if (close (STDIN_FILENO) < 0) {
      close (in_fd);
      syserrprintf ("close()");
      exit (get_exitstatus ());
   }
   if (dup (in_fd) != STDIN_FILENO) {
      close (in_fd);
      syserrprintf ("dup()");
      exit (get_exitstatus ());
   }
   close (in_fd);
}

void cmd_simple (cmdtree tree, cmdattr_ref cmdattr) {
   select_children (tree, cmdattr);
   int status;
   pid_t pid = fork ();
   if (pid == 0) { 
      if (cmdattr->output_r != NULL) set_output_redirect (cmdattr);
      if (cmdattr->input_r != NULL) set_input_redirect (cmdattr);
      int rc = execvp (cmdattr->lprog, cmdattr->largs); 
      if (rc == -1) {
         errprintf ("%:%s\n", strerror (errno));
         exit (get_exitstatus ());
      }
   }else if (pid > 0) {
      if (! yyparse_cmdbg_flag) wait (&status);
   }else {
      syserrprintf ("fork()");
   }
}

void cmd_select (cmdtree tree, cmdattr_ref cmdattr) {
   switch (tree->symbol) {
      case SIMPL: cmd_simple (tree, cmdattr);  break;
      case PIPE:  cmd_pipe   (tree, cmdattr);  break;
      case LEFT:  cmd_left   (tree, cmdattr);  break;
      case RIGHT: cmd_right  (tree, cmdattr);  break;
      case INVOC: cmd_invoc  (tree, cmdattr);  break;
      case PROG:  cmd_prog   (tree, cmdattr);  break;
      case ARGS:  cmd_args   (tree, cmdattr);  break;
      case REIN:  cmd_rein   (tree, cmdattr);  break;
      case REOUT: cmd_reout  (tree, cmdattr);  break;
      default: break;
   }
}

void execute (void) {
   cmdtree child = yyparse_cmdtree->first;
   for (;;) {
      if (child == NULL) break;
      cmdattr_ref cmdattr = new_cmdattr ();
      cmd_select (child, cmdattr); 
      free_cmdattr (cmdattr);
      child = child->next;
   }
}

void set_options (int argc, char **argv) {
   yy_flex_debug = 0;
   yydebug = 0;
   opterr = 0;
   for (;;) {
      int opt = getopt (argc, argv, "@:ly");
      if (opt == EOF) break;
      
      switch (opt) {
         char optopt_str[16];
         case '@': set_debugflags (optarg);                      
                   break;
         case 'l': yy_flex_debug = 1;                            
                   break;
         case 'y': yydebug = 1;                                  
                   break;
         default : sprintf (optopt_str, "-%c", optopt);
                   errprintf ("%:%c: invalid option\n", optopt_str);  
                   exit (get_exitstatus ());
      }
   }
}

int main (int argc, char **argv) {
   set_execname (argv[0]);
   set_options (argc, argv);
   for (;;) {
      printf ("mysh$ ");
      int yyparse_rc = yyparse ();
      //dump_cmdtree (yyparse_cmdtree);
      if (yyparse_rc == 0) execute ();
   }
   return get_exitstatus ();
}
