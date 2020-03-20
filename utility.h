#pragma once
/*
 * Structure to hold a resource request structure
 */
typedef struct resource_request_s {

    unsigned int memory;
    unsigned int num_printers;
    unsigned int num_scanners;
    unsigned int num_modems;
    unsigned int num_cds;

} resource_request;
