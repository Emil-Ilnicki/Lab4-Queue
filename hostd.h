#pragma once

// Standard libraries included in the header definition
#define _POSIX_SOURCE
#include <stdbool.h>
#include "utility.h"
#include "queue.h"

// Global variables used in the hostd.c program file
#define MEMORY 1024
#define MAX_PRINTERS 2
#define MAX_SCANNERS 1
#define MAX_MODEMS 1
#define MAX_CDS 2

/**
 * Resource structure to hold the total resources of the system being mimicked
 */
struct resources_s {
    bool avail_mem[MEMORY];
    unsigned int avail_printers;
    unsigned int avail_scanners;
    unsigned int avail_modems;
    unsigned int avail_cds;
};

// Define a structure resources, and a constant default-resources structure
typedef struct resources_s resources;
extern const resources resources_default; 

// Header definition for functions defined in hostd.c
int request_resources(resources* avail_resources, resource_request request, bool realtime);
void free_resources(resources* avail_resources, proc* p);