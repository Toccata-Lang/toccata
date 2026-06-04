
#include "new.h"
#include <string.h>

char *dupLabels[] = {};
unsigned otherNodes;
unsigned subGraphs = 0;
long graphCount = 0;

char *nodeFormat =  "x%x [label=\"%d\",  height=0.4, width=0.4, fixedsize=true,  shape=triangle, orientation=%d]\n";
char *noOutline =  "x%d_%x [label=\"%s\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n";
char *nodeXlblFormat =  "x%d_%x [label=\"%s\",  height=0.4, width=0.4, fixedsize=true,  shape=triangle, orientation=%d, xlabel=\"%s\"]\n";
char *eraseFormat =  "x%d_%x [label=\"\", height=0.1, width=0.1, color=black, fixedsize=true,  shape=circle, style=filled]\n";

char *nodeLabels[25] = {"V", " ", " ", " ", " ", "L", "A", "F", "V", "S", "D",
                        " ", " ", "#", "#", "Z"};

typedef struct graphNode {
  Term trm;
  Location node;
  struct graphNode *left;
  struct graphNode *right;
  Location lazyAPP;
} graphNode;

graphNode nodeStack[NODE_STACK_SIZE];

unsigned graphSubDown(unsigned graphNum, unsigned nodeNum, Term tree);
void downBranch(Term tree, unsigned pt, unsigned graphNum, unsigned nodeNum) {
  Tag t = termTag(tree);
  Location treeLoc = termLoc(tree);
  char *branchPort = pt == 1 ? "sw" : "se";
  Location loc = portLoc(pt, treeLoc);
  Term branch = get(loc);
  while (termTag(branch) == VAR) {
    Term val = get(termLoc(branch));
    if (val != SUB) {
      branch = val;
    } else {
      // BOOM("Draw an edge here");
      return;
    }
  }

  if (branch == SUB) {
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      Term left = get(portLoc(1, gn->node));
      Term right = get(portLoc(2, gn->node));
      if (termTag(left) == VAR && termLoc(left) == treeLoc) {
	fprintf(dotFile, "x%d_%x:sw -- x%d_%x:sw\n", graphNum, gn->node, graphNum, treeLoc);
	break;
      } else if (termTag(right) == VAR && termLoc(right) == treeLoc) {
	fprintf(dotFile, "x%d_%x:se -- x%d_%x:sw\n", graphNum, gn->node, graphNum, treeLoc);
	break;
      }
    }
    return;
  }

  unsigned branchNode = 65536;
  for (unsigned i = 0; i < nodeCount; i++) {
    graphNode *gn = &nodeStack[i];
    if (gn->trm == branch) {
      branchNode = gn->node;
      break;
    }
  }
  if (branchNode == 65536)
    branchNode = graphSubDown(graphNum, loc, branch);

  Tag bt = termTag(branch);
  Lab l = termLab(tree);

  if (pt == 2 && (t == APP || t == OPX || t == OPY) && bt == LAZ) {
    fprintf(dotFile, "x%d_%x:s -- x%d_%x:se\n", graphNum, branchNode, graphNum, nodeNum);
  } else {
    if (branchNode != 65536 ) {
      if (t == DUP && (bt != LAZ || get(portLoc(1, termLoc(branch))) != tree)) {
	fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		graphNum, nodeNum, branchPort, graphNum, branchNode);
      } else if (t == DUP && (bt != LAZ || get(portLoc(1, termLoc(branch))) == tree)) {
	return;
      } else {
	if (bt == VAR) {
	  Location branchLoc = termLoc(branch);
	  if (termTag(get(branchLoc)) == SUB || termTag(get(branchLoc)) == LAZ) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (branchLoc & 0xFFFFFFFE),
		    (branchLoc & 1) ? "se" : "sw");
	  } else if (termTag(get(branchLoc)) == LAZ &&
		     termTag(get(portLoc(1, termLoc(get(branchLoc))))) == DUP) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (branchLoc & 0xFFFFFFFE),
		    (branchLoc & 1) ? "se" : "sw");
	  } else {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		    graphNum, nodeNum, branchPort, graphNum, branchNode);
	  }
	} else {
	  fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		  graphNum, nodeNum, branchPort, graphNum, branchNode);
	}
      }
    }
  }
}

// graph the node and the tree under it, if needed. Return the node number
unsigned graphSubDown(unsigned graphNum, unsigned nodeNum, Term tree) {
  char xLbl[100];
  Tag t = termTag(tree);

  if (tree == SUB) {
    return 65536;
  } else if (hasLocation(tree)) {
    unsigned treeNode = termLoc(tree) & 0xFFFFFFFE;
    for (unsigned i = 0; i < nodeCount; i++) {
      graphNode *gn = &nodeStack[i];
      if (gn->node == treeNode) {
	if (nodeNum == 65536) {
	  while (termTag(tree) == VAR) {
	    tree = get(termLoc(tree));
	    treeNode = termLoc(tree) & 0xFFFFFFFE;
	  }
	  fprintf(dotFile, "x%d_0 [label=\"\", shape=plaintext, height=0, width=0, peripheries=0]\n",
		  graphNum);
	  fprintf(dotFile, "{rank=min; x%d_0;}\n", graphNum);
	  fprintf(dotFile, "x%d_0:s -- x%d_%x:n\n", graphNum, graphNum, treeNode);
	  return 0;
	}
	return gn->node;
      }
    }
    nodeNum = treeNode;
  } else {
    nodeNum = otherNodes++;
  }

  Lab lab = termLab(tree);
  switch(t) {
  case VAR: {
    Location loc = termLoc(tree);
    Term trm = get(loc);
    if (trm == SUB) {
      return 65536;
    } else if (termTag(trm) != LAZ || termTag(get(portLoc(1, termLoc(trm)))) != DUP) {
      return graphSubDown(graphNum, nodeNum, trm);
    } else {
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	if (hasLocation(gn->trm) && termLoc(gn->trm) == (loc & 0xFFFFFFFE))
	  return gn->node;
      }
      return graphSubDown(graphNum, nodeNum, trm);
    }
  }
    break;

  case ERA:
    if (termLab(tree) == 1)
      fprintf(dotFile, noOutline, graphNum, nodeNum, "SE");
    else
      fprintf(dotFile, eraseFormat, graphNum, nodeNum);
    break;

  case I60:
    snprintf(xLbl, 95, "%ld", getI60(tree));
    fprintf(dotFile, noOutline, graphNum, nodeNum, xLbl);
    break;

  case VAL:
    if (tree == VOID)
      fprintf(dotFile, noOutline, graphNum, nodeNum, "VOID");
    /*
    else if (((Value *)tree)->type == StringBufferType) {
      int len = (int)((String *)tree)->len;
      len = len > 10 ? 10 : len;
      fprintf(dotFile, "x%d_%x [label=\"'%-.*s'\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n",
	      graphNum, nodeNum, len, ((String *)tree)->buffer);
    } else if (((Value *)tree)->type == SubStringType) {
      ReifiedVal *ss = (ReifiedVal *)tree;
      String *parent = (String *)ss->impls[0];
      long start = getI60(ss->impls[1]);
      int len = (int)getI60(ss->impls[2]);
      len = len > 10 ? 10 : len;
      fprintf(dotFile, "x%d_%x [label=\"'%-.*s'\",  height=0.4, width=0.4, fixedsize=true, shape=plaintext]\n",
	      graphNum, nodeNum, len, &parent->buffer[start]);
    } else
    */
      fprintf(dotFile, noOutline, graphNum, nodeNum, nodeLabels[t]);
    break;

  case REF: {
    char *refName = "REF2";
    for (unsigned i = 0; i <= refsCount; i++) {
      if (refNames[i].fn == (interactionFn)(tree & ~7)) {
	refName = refNames[i].name;
	break;
      }
    }
    fprintf(dotFile,   "x%d_%x [label=\"%s\", shape=plaintext]\n",
	    graphNum, nodeNum, refName);
  }
    break;

  case SUB: {
    fprintf(dotFile, "x%d_%x [label=\"%s\",  height=0.22, width=0.22, fixedsize=true,  shape=circle]\n",
	    graphNum, nodeNum, "");
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    downBranch(tree, 1, graphNum, nodeNum);
    downBranch(tree, 2, graphNum, nodeNum);
  }
    break;

  case SUP:
  case DUP: 
  case OPX:
  case OPY:
  case LAZ:
  case LAM:
  case APP: {
    Lab lab = termLab(tree);
    if (t == DUP || t == SUP || t == LAM) {
      if (lab == 0 || strlen(dupLabels[lab]) == 0)
	snprintf(xLbl, 95, "%x:\n%d", nodeNum, lab);
      else
	snprintf(xLbl, 95, "%x:\n%s", nodeNum, dupLabels[lab]);
    } else
      snprintf(xLbl, 95, "%x:", nodeNum);

    if (t == OPX || t == OPY) {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "-", 0, xLbl);
    } else {
      fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, nodeLabels[t], 0, xLbl);
    }
    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = tree;
    gn->node = nodeNum;

    if (t == DUP) {
      Location dLoc = termLoc(tree);
      Term b1 = get(portLoc(1, dLoc));
      Tag bt1 = termTag(b1);
      if (bt1 != LAZ || tree == get(portLoc(1, termLoc(b1)))) {
	downBranch(tree, 1, graphNum, nodeNum);
      }

      Term b2 = get(portLoc(2, dLoc));
      Tag bt2 = termTag(b2);
      if (bt2 != LAZ || tree == get(portLoc(2, termLoc(b2)))) {
	downBranch(tree, 2, graphNum, nodeNum);
      }
    } else {
      downBranch(tree, 1, graphNum, nodeNum);
      downBranch(tree, 2, graphNum, nodeNum);
    }
  }
    break;
    
  default: {
    fprintf(dotFile,   "x%d_%x [label=\"%s\", shape=plaintext]\n",
	    graphNum, nodeNum, tagStr(t));
  }
    break;
    
  }
  return nodeNum;
}

unsigned graphDown(char *title, Term root, unsigned currNodeCount, unsigned graphNum) {
  char xLbl[100];
  nodeCount = currNodeCount;
  fprintf(dotFile, "subgraph cluster%d {\ngraph [color=none, label=\"%s\"]\n", subGraphs++, title);

  if (currNodeCount == 0)
    otherNodes = buffEnd;

  //*
  if (termTag(root) == LAM) {
    unsigned nodeNum = otherNodes++;
    unsigned rootNode = termLoc(root);
    Lab rootLab = termLab(root);
    if (rootLab == 0 || strlen(dupLabels[rootLab]) == 0)
      snprintf(xLbl, 95, "%x:\n%d", rootNode, rootLab);
    else
      snprintf(xLbl, 95, "%x:\n%s", rootNode, dupLabels[rootLab]);
    fprintf(dotFile, nodeXlblFormat, graphNum, nodeNum, "L", 0, xLbl);
    fprintf(dotFile, "{rank=min; x%d_%x;}\n", graphNum, nodeNum);

    graphNode *gn = &nodeStack[nodeCount++];
    if (nodeCount > 999)
      BOOM("nodeCount!");
    gn->trm = root;
    gn->node = rootNode;

    Term left = get(portLoc(1, rootNode));
    if (termTag(left) != SUB) {
      unsigned leftNode = graphSubDown(graphNum, 65536, left);
      fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, leftNode);
    } else {
      fprintf(dotFile, "x%d_%x [label=\"\", shape=plaintext, height=0, width=0, peripheries=0]\n",
	      graphNum, rootNode);
      fprintf(dotFile, "{rank=max; x%d_%x;}\n", graphNum, rootNode);

      char foundVar = 0;
      for (unsigned i = 0; i < nodeCount; i++) {
	graphNode *gn = &nodeStack[i];
	Term left = get(portLoc(1, gn->node));
	Term right = get(portLoc(2, gn->node));
	if (termTag(left) == VAR && termLoc(left) == rootNode) {
	  fprintf(dotFile, "x%d_%x:sw -- x%d_%x:sw\n", graphNum, gn->node, graphNum, nodeNum);
	  foundVar = 1;
	  break;
	} else if (termTag(right) == VAR && termLoc(right) == rootNode) {
	  fprintf(dotFile, "x%d_%x:se -- x%d_%x:sw\n", graphNum, gn->node, graphNum, nodeNum);
	  foundVar = 1;
	  break;
	}
      }
      if (!foundVar) {
	fprintf(dotFile, "x%d_%x:sw -- x%d_%x\n", graphNum, nodeNum, graphNum, rootNode);
      }
    }

    downBranch(root, 2,graphNum, nodeNum);
    fprintf(dotFile, "}\n");
    return nodeNum;
  } else
  // */
  {
    unsigned rootNode = graphSubDown(graphNum, 65536, root);
    fprintf(dotFile, "}\n");
    return rootNode;
  }
}

/*
// graphFn - native graphing function
// Needs: sideEffects, argsNet, strictArgs, makePair
void graphFn(Term ref, Term args) {
  NativeArgs argsStruct = {0, {}};
  args = strictArgs(ref, args, 1, &argsStruct);
  if (argsStruct.count != 1) {
    return;
  }

  args = take(portLoc(2, termLoc(args))); 
  Term arg = take(portLoc(1, termLoc(args))); 

  if (termTag(arg) == VAR) {
    Term val = get(termLoc(arg));
    switch(termTag(val)) {
    case LAZ:
      swap(termLoc(arg), SUB);
      forceLazy(val);

    case SUB:
      // add the remaining args to argsStruct
      argsStruct.args[argsStruct.count++] = args;

      // create a chain of APP terms from argsStruct
      Term newArgs = argsNet(&argsStruct);

      // put 'arg' back in it's place
      swap(portLoc(1, termLoc(args)), arg);

      // make a deferred redex to retry the APP/REF pair when the value becomes available
      Term retry = makePair(SUB, 5, newArgs, ref);

      // and put it in the location 'arg' points to
      Term newArg = swap(termLoc(arg), retry);
      if (newArg != SUB) {
        // someone slipped the needed arg in since we last looked
        swap(termLoc(arg), newArg);
        freePair(termLoc(retry));

        // so retry the original APP/REF redex
        pushRedex(newArgs, ref);
      }
      break;

    default:
      printRawTerm(val);
      printf("\n");
      BOOM("nativeArgs");
      break;
    }
  } else {
    String *s = (String *)argsStruct.args[0];
    char cap[200];
    sprintf(cap, "%-.*s", (int)((String *)s)->len, ((String *)s)->buffer);
    graphDown(cap, arg, 0, subGraphs++);
    move(portLoc(2, termLoc(args)), arg);
  }
}
*/
