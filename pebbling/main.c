//
//  main.c
//  pebbling
//
//  Created by Mattia Sgrò on 27/01/26.
//

#include "gfa.h"
#include "peb.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <gfa_file>\n", argv[0]);
    return EXIT_FAILURE;
  }
  gfa_t *ingfa = gfa_read(argv[1]);
  if (!ingfa) {
    fprintf(stderr, "Failed to parse GFA file\n");
    return EXIT_FAILURE;
  }
  size_t i, j; // i j indexes to check
  if (argc > 3) {
    i = (size_t)atoi(argv[2]);
    j = (size_t)atoi(argv[3]);
  } else {
    i = 0;
    j = 1;
  }
  fprintf(stdout, "GFA file '%s' successfully parsed.\n", argv[1]);
  fprintf(stdout, "Graph has %u segments and %lu arcs.\n", ingfa->n_seg,
          ingfa->n_arc);

  fprintf(stdout,
          "Iterating over paths in the graph... n_path: %u, n_walk: %u\n",
          ingfa->n_path, ingfa->n_walk);

  if (ingfa->n_path > 0 && ingfa->path) {
    for (size_t i = 0; i < ingfa->n_path; i++) {
      gfa_walk_t *p = &ingfa->path[i];
      fprintf(stdout, "Path %u: n_v=%u\n", (uint32_t)i, p->n_v);
      if (p->v) {
        for (size_t j = 0; j < p->n_v; ++j) {
          uint32_t seg_id = p->v[j] >> 1;
          if (seg_id < ingfa->n_seg) {
            fprintf(stdout, "\tSegment %s (ID: %u, Orientation: %s)\n",
                    ingfa->seg[seg_id].name, seg_id,
                    p->v[j] & 1 ? "reverse" : "forward");
          }
          gfa_arc_t *av = gfa_arc_a(ingfa, p->v[j]);
          int nv = gfa_arc_n(ingfa, p->v[j]);
          for (int i = 0; i < nv; ++i) {
            gfa_arc_t *avi = &av[i];
            if (p->v[j] & 1) {
              printf("\t- h:%d [%d, %s] (rev)-> t:%d [%d, %s]\n",
                     gfa_arc_head(*avi), // gfatools index, accounts also for
                                         // direction (last bit)
                     gfa_arc_head(*avi) >> 1, // real index
                     ingfa
                         ->seg[gfa_arc_head(*avi) >>
                               1] // real name, retrive from the real index
                         .name,   // index is also: avi->v_lv>>33
                     gfa_arc_tail(*avi), gfa_arc_tail(*avi) >> 1,
                     ingfa->seg[gfa_arc_tail(*avi) >> 1]
                         .name // index is also avi->w>>1
              );

            } else {
              printf("\t- h:%d [%d, %s] -> t:%d [%d, %s]\n",
                     gfa_arc_head(*avi), // gfatools index, accounts also for
                                         // direction (last bit)
                     gfa_arc_head(*avi) >> 1, // real index
                     ingfa
                         ->seg[gfa_arc_head(*avi) >>
                               1] // real name, retrive from the real index
                         .name,   // index is also: avi->v_lv>>33
                     gfa_arc_tail(*avi), gfa_arc_tail(*avi) >> 1,
                     ingfa->seg[gfa_arc_tail(*avi) >> 1]
                         .name // index is also avi->w>>1
              );
            }
          }
        }
      }
    }
  } else {
    fprintf(stdout, "No paths found in graph.\n");
  }

  // if (ingfa->n_walk > 0 && ingfa->walk) {
  //   for (size_t i = 0; i < ingfa->n_walk; i++) {
  //     gfa_walk_t *w = &ingfa->walk[i];
  //     fprintf(stdout, "Walk %u: n_v=%u\n", (uint32_t)i, w->n_v);
  //     if (w->v) {
  //       for (size_t j = 0; j < w->n_v; ++j) {
  //         uint32_t seg_id = w->v[j] >> 1;
  //         if (seg_id < ingfa->n_seg) {
  //           fprintf(stdout, "\tSegment %s (ID: %u, Orientation: %s)\n",
  //                   ingfa->seg[seg_id].name, seg_id,
  //                   w->v[j] & 1 ? "reverse" : "forward");
  //         }
  //       }
  //     }
  //   }
  // } else {
  //   fprintf(stdout, "No walks found in graph.\n");
  // }

  int8_t **pebbles = NULL;
  size_t peb_n = 0;
  minsp(ingfa, &pebbles, &peb_n);

  gfa_destroy(ingfa);
  return EXIT_SUCCESS;
}
