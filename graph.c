
#include "new.h"
#include <string.h>

unsigned labelsCount = 0;
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
unsigned downBranch(Term tree, unsigned pt, unsigned graphNum, unsigned nodeNum) {
  Tag t = termTag(tree);
  Location treeLoc = termLoc(tree);
  char *branchPort = pt == 1 ? "sw" : "se";
  Location loc = portLoc(pt, tree);
  Term branch = get(loc);
  while (termTag(branch) == VAR) {
    Term val = get(termLoc(branch));
    if (val != SUB) {
      branch = val;
    } else {
      fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
	      graphNum, nodeNum, branchPort, graphNum, termLoc(branch),
	      termLoc(branch) & -1 ? "sw" : "se");
      return termLoc(branch);
    }
  }

  if (branch == SUB) {
    return 65536;
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
      if (t == DUP && (bt != LAZ || get(portLoc(1, branch)) != tree)) {
	fprintf(dotFile, "x%d_%x:%s -- x%d_%x:n\n",
		graphNum, nodeNum, branchPort, graphNum, branchNode);
      } else if (t == DUP && (bt != LAZ || get(portLoc(1, branch)) == tree)) {
	if (hasLocation(branch))
	  return branchNode;
	else
	  return 65536;    
      } else {
	if (bt == VAR) {
	  Location branchLoc = termLoc(branch);
	  if (termTag(get(branchLoc)) == SUB || termTag(get(branchLoc)) == LAZ) {
	    fprintf(dotFile, "x%d_%x:%s -- x%d_%x:%s\n",
		    graphNum, nodeNum, branchPort, graphNum, (branchLoc & 0xFFFFFFFE),
		    (branchLoc & 1) ? "se" : "sw");
	  } else if (termTag(get(branchLoc)) == LAZ &&
		     termTag(get(portLoc(1, get(branchLoc)))) == DUP) {
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
  if (hasLocation(branch))
    return branchNode;
  else
    return 65536;    
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
    } else if (termTag(trm) != LAZ || termTag(get(portLoc(1, trm))) != DUP) {
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
      if (lab == 0 || lab >= labelsCount || strlen(dupLabels[lab]) == 0)
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

    unsigned leftNode = 65536;
    unsigned rightNode = 65536;
    if (t == DUP) {
      Term b1 = get(portLoc(1, tree));
      Tag bt1 = termTag(b1);
      if (bt1 != LAZ || tree == get(portLoc(1, b1))) {
	leftNode = downBranch(tree, 1, graphNum, nodeNum);
      }

      Term b2 = get(portLoc(2, tree));
      Tag bt2 = termTag(b2);
      if (bt2 != LAZ || tree == get(portLoc(2, b2))) {
	rightNode = downBranch(tree, 2, graphNum, nodeNum);
      }
    } else {
      leftNode = downBranch(tree, 1, graphNum, nodeNum);
      rightNode = downBranch(tree, 2, graphNum, nodeNum);
    }
    if (leftNode != 65536 && rightNode != 65536)
      fprintf(dotFile, "rank=same {x%d_%x, x%d_%x}\n",
	      graphNum, leftNode, graphNum, rightNode);
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
    Location rootNode = termLoc(root);
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

    Term left = get(portLoc(1, root));
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
	Term left = get(gn->node);
	Term right = get(gn->node + 1);
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

void subGraph(char *cap, Term pos, unsigned nc) {
  graphDown(cap, pos, nc, subGraphs++);
}

void graph(char *cap, Term pos, unsigned nc) {
  graphDown(cap, pos, nc, 0);
}
