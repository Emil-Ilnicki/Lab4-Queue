#pragma once

typedef struct resource_request_s {

    unsigned int memory;
    unsigned int num_printers;
    unsigned int num_scanners;
    unsigned int num_modems;
    unsigned int num_cds;
    int mem_ptr;

} resource_request;
