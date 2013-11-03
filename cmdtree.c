
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlib.h"
#include "cmdtree.h"
#include "lyutils.h"

static char *cmdtree_tag = "struct cmdtree_rep";

bool is_cmdtree (void *object) {
   cmdtree tree = (cmdtree) object;
   return tree != NULL && tree->tag == cmdtree_tag;
}

cmdtree new_cmdtree (int symbol, char *lexinfo) {
   cmdtree tree = malloc (sizeof (struct cmdtree_rep));
   assert (tree != NULL);
   tree->tag = cmdtree_tag;
   tree->lexinfo = strdup (lexinfo);
   tree->symbol = symbol;
   tree->first = NULL;
   tree->last = NULL;
   tree->next = NULL;
   DEBUGF ('c', "tree=%p; sym=%d; lexinfo=\"%s\"", tree,
           tree->symbol, tree->lexinfo);
   return tree;
}

cmdtree adopt (cmdtree root, ...) {
   assert (is_cmdtree (root));
   va_list children;
   va_start (children, root);
   for (;;) {
      cmdtree child = va_arg (children, cmdtree);
      if (child == NULL) break;

      assert (is_cmdtree (child));
      if (root->last == NULL) root->first = child; 
                         else root->last->next = child;
      root->last = child;
   }
   va_end (children);
   return root;
}

cmdtree adopt1 (cmdtree root, cmdtree child) {
   return adopt (root, child, NULL);
}

cmdtree adopt2 (cmdtree root, cmdtree left, cmdtree right) {
   return adopt (root, left, right, NULL);
}

cmdtree adopt1sym (cmdtree root, cmdtree child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

cmdtree adopt2sym (cmdtree root, cmdtree left, 
                   cmdtree right, int symbol) {
   root = adopt2 (root, left, right);
   root->symbol = symbol;
   return root;
}

static void dump_node (cmdtree node, int depth) {
   assert (is_cmdtree (node));
   if (node->symbol != CMD_ROOT) {
      for (int itor = 0; itor < depth; ++itor) {
         printf ("|  ");
      }
   }
   const char *tname = get_yytname (node->symbol);
   printf ("%s \"%s\"\n", tname, node->lexinfo);
}

static void dump_cmdtree_rec (cmdtree root, int depth) {
   if (root == NULL) return;
   assert (is_cmdtree (root));
   dump_node (root, depth);
   cmdtree child = root->first;
   for (;;) {
      if (child == NULL) break;
      dump_cmdtree_rec (child, depth + 1);
      child = child->next;
   }   
}

void dump_cmdtree (cmdtree root) {
   dump_cmdtree_rec (root, 0);
   fflush (NULL);
}

void freecmd (cmdtree root) {
   DEBUGF ('f', "root=%p\n");
   assert (is_cmdtree (root));
   if (root == NULL) return;
   cmdtree child = root->first;
   for (;;) {
      cmdtree old = child;
      freecmd (old);
   }
   free (root->lexinfo);
   memset (root, 0, sizeof (struct cmdtree_rep));
   free (root);
}
