#pragma once

#define _POSIX_SOURCE
#include <stdbool.h>
#include "utility.h"
#include "queue.h"

#define MEMORY 1024
#define MAX_PRINTERS 2
#define MAX_SCANNERS 1
#define MAX_MODEMS 1
#define MAX_CDS 2

struct resources_s {
    bool avail_mem[MEMORY];
    unsigned int avail_printers;
    unsigned int avail_scanners;
    unsigned int avail_modems;
    unsigned int avail_cds;
};

typedef struct resources_s resources;

extern const resources resources_default; 

int request_resources(resources* avail_resources, resource_request request, bool realtime);
void free_resources(resources* avail_resources, proc* p);