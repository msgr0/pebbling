#ifndef PEB_H
#define PEB_H
#include "gfa.h"
#include <stdio.h>

// st-DAG structure with virtual source and sink nodes
// Extracted from path definitions in GFA file:
// - sources: first nodes of each path (connected from virtual source s)
// - sinks: last nodes of each path (connected to virtual sink t)
typedef struct {
  uint32_t *sources; // first nodes of paths
  uint32_t n_sources;
  uint32_t *sinks; // last nodes of paths
  uint32_t n_sinks;
} st_dag_t;

// Convert graph to st-DAG by extracting sources/sinks from GFA paths
st_dag_t *gfa_to_st_dag(gfa_t *gfa);
void free_st_dag(st_dag_t *dag);

// MULtiPathCounting:
// given a graph gfa and a conmat[NxN]
// compute in conmat[NxN] for (i,j) how many (if at least 2 simple paths)
// are present between (i, j)

// do not account for cycles for now.

int32_t mulpc(gfa_t *gfa, int8_t **c, int8_t **v, uint32_t i, uint32_t j);
// MinimalSizePebbling
void minsp(gfa_t *gfa, int8_t ***peb, size_t *peb_n);

#endif // PEB_H