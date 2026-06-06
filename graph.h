#ifndef GRAPH_H
#define GRAPH_H

extern FILE *dotFile;

extern unsigned subGraphs;
unsigned subGraph(char *title, Term root, unsigned currNodeCount);
unsigned graph(char *title, Term root, unsigned currNodeCount);

#endif
