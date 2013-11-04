
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

typedef struct cmd_attributes *cmdattr;

void cmd_select (cmdtree, cmdattr);

struct cmd_attributes {
   char *lprog;
   char **largs;
   char *rprog;
   char **rargs;
   char *input_r;
   char *output_r;
};

cmdattr new_cmdattr (void) {
   cmdattr new = malloc (sizeof (struct cmd_attributes));
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

void free_cmdattr (cmdattr attr) {
   free (attr->largs);
   free (attr->rargs);
   free (attr);
}

void select_children (cmdtree tree, cmdattr attr) {
   cmdtree child = tree->first;
   for (;;) {
      if (child == NULL) break;
      cmd_select (child, attr);
      child = child->next;
   }
}

void cmd_reout (cmdtree tree, cmdattr attr) {
   attr->output_r = tree->first->lexinfo;
}

void cmd_rein (cmdtree tree, cmdattr attr) {
   attr->input_r = tree->first->lexinfo;
}

void cmd_args (cmdtree tree, cmdattr attr) {
   cmdtree child = tree->first;
   int ind = 1;
   while (ind < MAXLEN) {
      if (child == NULL) break; 
      attr->largs[ind++] = child->lexinfo;
      child = child->next;
   }
   attr->largs[ind] = NULL;
}

void cmd_prog (cmdtree tree, cmdattr attr) {
   attr->lprog = tree->first->lexinfo;
   attr->largs[0] = tree->first->lexinfo;
}

void cmd_invoc (cmdtree tree, cmdattr attr) {
   select_children (tree, attr);
}

void cmd_right (cmdtree tree, cmdattr attr) {
   cmdtree invoc = tree->first;
   attr->rprog = invoc->first->first->lexinfo;
   attr->rargs[0] = attr->rprog;
   cmdtree args = invoc->first->next;
   int ind = 1;
   if (args != NULL) {
      cmdtree arg = args->first;
      while (ind < MAXLEN) {
         if (arg == NULL) break; 
         attr->rargs[ind++] = arg->lexinfo;
         arg = arg->next;
      }
   }
   attr->rargs[ind] = NULL;
   cmdtree reout = invoc->next;
   if (reout != NULL) cmd_select (reout, attr);
}

void cmd_left (cmdtree tree, cmdattr attr) {
   select_children (tree, attr);
}


void set_output_redirect (cmdattr attr) {
   int out_fd = -1;
   out_fd = open (attr->output_r, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
   if (out_fd < 0) {
      syserrprintf (attr->output_r);
      exit (get_exitstatus ());
   }
   if (close (STDOUT_FILENO) < 0) {
      close (out_fd);
      syserrprintf ("close(2)");
      exit (get_exitstatus ());
   }
   if (dup (out_fd) != STDOUT_FILENO) {
      close (out_fd);
      syserrprintf ("dup(2)");
      exit (get_exitstatus ());
   }
   close (out_fd);
}

void set_input_redirect (cmdattr attr) {
   int in_fd = -1;
   in_fd = open (attr->input_r, O_RDONLY);
   if (in_fd < 0) {
      syserrprintf (attr->input_r);
      exit (get_exitstatus ());
   }
   if (close (STDIN_FILENO) < 0) {
      close (in_fd);
      syserrprintf ("close(2)");
      exit (get_exitstatus ());
   }
   if (dup (in_fd) != STDIN_FILENO) {
      close (in_fd);
      syserrprintf ("dup(2)");
      exit (get_exitstatus ());
   }
   close (in_fd);
}


void execvp_cmd (char *prog, char **argv) {
   int rc = execvp (prog, argv); 
   if (rc == -1) {
      errprintf ("%:%s\n", strerror (errno));
      exit (get_exitstatus ());
   }
}

void exec_pipe (cmdattr attr) {
   int pipefd[2];
   pipe (pipefd);
   pid_t pid = fork ();
   if (pid == 0) { 
      close (STDOUT_FILENO);
      dup (pipefd[1]);
      close (pipefd[0]);
      close (pipefd[1]);
      if (attr->input_r != NULL) set_input_redirect (attr);
      execvp_cmd (attr->lprog, attr->largs);
   }else if (pid > 0) {
      close (STDIN_FILENO);
      dup (pipefd[0]);
      close (pipefd[0]);
      close (pipefd[1]);
      if (attr->output_r != NULL) set_output_redirect (attr);
      execvp_cmd (attr->rprog, attr->rargs);
   }else {
      syserrprintf ("fork(2)");
      exit (get_exitstatus ());
   }
}

void exec_simple (cmdattr attr) {
   if (attr->output_r != NULL) set_output_redirect (attr);
   if (attr->input_r != NULL) set_input_redirect (attr);
   execvp_cmd (attr->lprog, attr->largs);  
}

void cmd_exec (cmdtree tree, cmdattr attr) {
   select_children (tree, attr);
   int status;
   pid_t pid = fork ();
   if (pid == 0) { 
      if (tree->symbol == SIMPL) {
         exec_simple (attr);
      }else {
         exec_pipe (attr);
      }
   }else if (pid > 0) {
      if (! yyparse_cmdbg_flag) wait (&status);
   }else {
      syserrprintf ("fork(2)");
   }
}

void cmd_select (cmdtree tree, cmdattr attr) {
   switch (tree->symbol) {
      case SIMPL: cmd_exec   (tree, attr);  break;
      case PIPE:  cmd_exec   (tree, attr);  break;
      case LEFT:  cmd_left   (tree, attr);  break;
      case RIGHT: cmd_right  (tree, attr);  break;
      case INVOC: cmd_invoc  (tree, attr);  break;
      case PROG:  cmd_prog   (tree, attr);  break;
      case ARGS:  cmd_args   (tree, attr);  break;
      case REIN:  cmd_rein   (tree, attr);  break;
      case REOUT: cmd_reout  (tree, attr);  break;
      default: break;
   }
}

void execute (void) {
   cmdtree child = yyparse_cmdtree->first;
   for (;;) {
      if (child == NULL) break;
      cmdattr attr = new_cmdattr ();
      cmd_select (child, attr); 
      free_cmdattr (attr);
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
