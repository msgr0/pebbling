#include "peb.h"
#include "gfa-priv.h"
#include <stdlib.h>
#include <string.h>
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// Convert graph to st-DAG by identifying source and sink nodes
// Returns arrays of source and sink node indices

st_dag_t *gfa_to_st_dag(gfa_t *gfa) {
  st_dag_t *dag = malloc(sizeof(st_dag_t));

  // Allocate arrays for sources and sinks (max n_seg each)
  dag->sources = malloc(gfa->n_seg * sizeof(uint32_t));
  dag->sinks = malloc(gfa->n_seg * sizeof(uint32_t));
  dag->n_sources = 0;
  dag->n_sinks = 0;

  fprintf(stderr, "\n=== st-DAG: Extracting sources/sinks from paths ===\n");

  // Extract sources and sinks directly from path definitions
  if (gfa->n_path > 0) {
    for (size_t k = 0; k < gfa->n_path; ++k) {
      gfa_walk_t *path = &gfa->path[k];
      if (path->n_v < 1)
        continue;

      uint32_t start = path->v[0] >> 1;
      uint32_t end = path->v[path->n_v - 1] >> 1;

      // Add start node to sources if not already present
      int found = 0;
      for (uint32_t i = 0; i < dag->n_sources; i++) {
        if (dag->sources[i] == start) {
          found = 1;
          break;
        }
      }
      if (!found) {
        dag->sources[dag->n_sources++] = start;
        fprintf(stderr, "Path %zu: s -> %u\n", k, start);
      }

      // Add end node to sinks if not already present
      found = 0;
      for (uint32_t i = 0; i < dag->n_sinks; i++) {
        if (dag->sinks[i] == end) {
          found = 1;
          break;
        }
      }
      if (!found) {
        dag->sinks[dag->n_sinks++] = end;
        fprintf(stderr, "Path %zu: %u -> t\n", k, end);
      }
    }
  }

  return dag;
}

void free_st_dag(st_dag_t *dag) {
  if (dag) {
    free(dag->sources);
    free(dag->sinks);
    free(dag);
  }
}

// MultiPathCounting
int32_t mulpc(gfa_t *gfa, int8_t **c, int8_t **v, uint32_t i, uint32_t j) {

  // if c in not allocated, allocate it and initialize to -1
  // add s-t to the count
  int32_t _nseg = gfa->n_seg + 2;
  if (*c == NULL) {
    *c = calloc(_nseg * _nseg, sizeof(int8_t));
    for (uint32_t k = 0; k < _nseg * _nseg; k++) {
      (*c)[k] = -1;
    }

    // precompute s-t paths count
    for (uint32_t s = 0; s < gfa->n_path; s++) {
      gfa_walk_t *p = &gfa->path[s];
      if (p->n_v < 1)
        continue;
      uint32_t start = p->v[0] >> 1;
      uint32_t end = p->v[p->n_v - 1] >> 1;

      // s -> start: row=gfa->n_seg (s), col=start
      if (start >= gfa->n_seg || end >= gfa->n_seg)
        continue;

      int32_t idx_s_to_start = gfa->n_seg * _nseg + start;
      (*c)[idx_s_to_start] = 1;
      // if ((*c)[idx_s_to_start] == -1) {
      //   (*c)[idx_s_to_start] = 1;
      // } else {
      //   (*c)[idx_s_to_start]++;
      // }

      // end -> t: row=end, col=gfa->n_seg+1 (t)
      int32_t idx_end_to_t = end * _nseg + (gfa->n_seg + 1);
      (*c)[idx_end_to_t] = 1;

      // if ((*c)[idx_end_to_t] == -1) {
      //   (*c)[idx_end_to_t] = 1;
      // } else {
      //   (*c)[idx_end_to_t]++;
      // }
    }
  }
  // if v in not allocated, allocate it and initialize to 0
  if (*v == NULL) {
    *v = calloc(_nseg, sizeof(int8_t));
    for (uint32_t k = 0; k < _nseg; k++) {
      (*v)[k] = 0;
    }
  }

  int32_t idx = i * _nseg + j;
  if ((*c)[idx] != -1)
    return (*c)[idx];
  if (i == j) {
    (*c)[idx] = 1; // Cache base case
    return 1;
  }

  // Cannot compute paths from virtual nodes (they have no outgoing arcs in
  // graph)
  if (i >= gfa->n_seg) {
    (*c)[idx] = 0;
    return 0;
  }

  (*v)[i] = 1;
  int8_t count = 0;
  // iterate over the outgoing arcs of i (only for real nodes)
  uint32_t vid = i << 1;
  gfa_arc_t *outa_vid = gfa_arc_a(gfa, vid);
  uint32_t n_outa_vid = gfa_arc_n(gfa, vid);
  for (uint32_t k = 0; k < n_outa_vid; k++) {
    if (count > 1) {
      (*c)[idx] = 2; // Cache early saturation
      (*v)[i] = 0;
      return 2;
    }
    uint32_t w = outa_vid[k].w >> 1;
    if (!(*v)[w]) {
      count += mulpc(gfa, c, v, w, j);
    }
  }
  (*v)[i] = 0;
  if (count > 1) {
    count = 2;
  }
  (*c)[idx] = count;
  return count;
}

// MinimalSizePebbling
void minsp(gfa_t *gfa, int8_t ***peb, size_t *peb_n) {
  int8_t *c = NULL;
  int8_t *v = NULL;

  // if (peb == NULL || *peb == NULL) {
  //   // peb = malloc(sizeof(int8_t *));
  //   *peb = NULL;
  // }
  // if (peb_n == NULL) {
  //   peb_n = malloc(sizeof(size_t));
  //   *peb_n = 0;
  // }
  // (*peb_n)++;
  // *peb = realloc(*peb, (*peb_n) * sizeof(int8_t *));

  if (gfa->n_path > 0) {
    // peb[peb_n-1]; already allocated to corret size at beginning of function
    for (size_t k = 0; k < gfa->n_path; ++k) {

      gfa_walk_t p = gfa->path[k];
      uint32_t path_start = p.v[0] >> 1;
      uint32_t path_end = p.v[p.n_v - 1] >> 1;
      // iterate from sink to source
      int32_t pre = gfa->n_seg + 1; // sink
      gfa_arc_t *glist = malloc(p.n_v * sizeof(gfa_arc_t));
      size_t glist_n = 0;

      for (int32_t s = p.n_v - 1; s >= -1; --s) {
        uint32_t i;
        if (s == -1) { // SOURCE node at gfa->n_seg;
          i = gfa->n_seg;
        } else {
          i = p.v[s] >> 1; // curr
        }

        uint32_t j = pre;

        // Only call mulpc for real-to-real node pairs
        int32_t count_ij = -1;
        if (i < gfa->n_seg && j < gfa->n_seg + 2) {
          count_ij = mulpc(gfa, &c, &v, i, j);
        } else if (i >= gfa->n_seg || j >= gfa->n_seg + 2) {
          // Virtual node pairs - use precomputed matrix directly
          int32_t _nseg = gfa->n_seg + 2;
          int32_t idx = i * _nseg + j;
          if (c != NULL && idx >= 0 && idx < _nseg * _nseg) {
            count_ij = c[idx];
          }
        }
        fprintf(stdout, "-----Number of paths from %u to %u: %d\n", i, j,
                count_ij);
        if (count_ij > 1) {
          fprintf(stdout,
                  "-----Multiple paths detected from %u to %u. Adding pebble"
                  " to edge (%u -> %u).\n",
                  i, j, i, (s >= 0) ? (p.v[s + 1] >> 1) : 0);

          // take arc i-p.v[s+1] and add pebble to it (only for real nodes)
          if (s >= 0 && i < gfa->n_seg) {
            gfa_arc_t *outa_vid = gfa_arc_a(gfa, i << 1);
            uint32_t n_outa_vid = gfa_arc_n(gfa, i << 1);
            for (uint32_t k = 0; k < n_outa_vid; k++) {
              uint32_t w = outa_vid[k].w >> 1;
              if (w == (p.v[s + 1] >> 1)) {
                fprintf(stdout, "-----Found arc %u -> %u. Adding pebble.\n", i,
                        w);
                glist[glist_n] = outa_vid[k];
                glist_n++;
                break;
              }
            }
          }
          // found the arc i -> p.v[s+1]

          // append
          // peb[peb_n][] append (ij)
          pre = i;
        }
      }
      fprintf(stdout, "-----Pebbles for path %zu:\n", k);
      for (size_t m = 0; m < glist_n; m++) {
        fprintf(stdout, "  %u -> %u\n", glist[m].v_lv >> 33, // head
                glist[m].w >> 1                              // tail
        );
      }
      free(glist);
      // for (size_t s = 0; s < p.n_v - 1; ++s) {
      //   uint32_t i = p.v[s] >> 1;
      //   uint32_t j = p.v[s + 1] >> 1;

      //   int32_t count_ij = mulpc(gfa, &c, &v, i, j);
      // }
    }
  }
  free(c);
  free(v);
  // free_st_dag(dag);
}
