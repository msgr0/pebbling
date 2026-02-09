//
//  main.c
//  pebbling
//
//  Created by Mattia Sgrò on 27/01/26.
//

#include <stdlib.h>
#include <stdio.h>
#include "gfatools/gfa.h"

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <gfa_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    gfa_t *ingfa = gfa_read(argv[1]);
    if (!ingfa) {
        fprintf(stderr, "Failed to parse GFA file\n");
        return EXIT_FAILURE;
    }


    // gfa_print(ingfa, stdout, 0);
    gfa_destroy(ingfa);
    
    return EXIT_SUCCESS;
}
