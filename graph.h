#ifndef GRAPH_H
#define GRAPH_H

#include "new.h"
extern FILE *dotFile;

extern unsigned subGraphs;
unsigned subGraph(char *title, Term root, unsigned currNodeCount);
void graphFn(Term ref, Term args);
unsigned graph(char *title, Term root, unsigned currNodeCount);

#endif
