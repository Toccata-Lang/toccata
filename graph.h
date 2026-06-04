#ifndef GRAPH_H
#define GRAPH_H

extern FILE *dotFile;

extern unsigned subGraphs;
unsigned graphDown(char *title, Term root, unsigned currNodeCount, unsigned graphNum);

#endif
