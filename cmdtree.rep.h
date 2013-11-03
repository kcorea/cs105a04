
#ifndef __CMDTREE_REP_H__
#define __CMDTREE_REP_H__

typedef struct cmdtree_rep *cmdtree;

struct cmdtree_rep {
   char *tag;         // tag field to verify class membership
   char *lexinfo;     // pointer to lexical information
   int symbol;        // token code
   cmdtree first;     // first child node of this node
   cmdtree last;      // last child node of this node
   cmdtree next;      // next younger sibling of this node
};

#endif
