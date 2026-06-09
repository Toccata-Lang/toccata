#ifndef GRAPH_H
#define GRAPH_H

#include "new.h"
extern FILE *dotFile;

extern unsigned subGraphs;
extern long graphCount;
unsigned subGraph(char *title, Term root, unsigned currNodeCount);
unsigned graph(char *title, Term root, unsigned currNodeCount);

#endif
