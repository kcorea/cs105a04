
#ifndef __CMDTREE_H__
#define __CMDTREE_H__

#include "cmdtree.rep.h"

cmdtree new_cmdtree (int symbol, char *lexinfo);
cmdtree adopt1 (cmdtree root, cmdtree child);
cmdtree adopt2 (cmdtree root, cmdtree left, cmdtree right);
cmdtree adopt1sym (cmdtree root, cmdtree child, int symbol);
cmdtree adopt2sym (cmdtree root, cmdtree left, 
                   cmdtree right, int symbol);
void dump_cmdtree (cmdtree root);
void freecmd (cmdtree tree);

#endif
