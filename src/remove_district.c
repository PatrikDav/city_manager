#include <stdio.h>
#include "remove_district.h"

/*
 * remove_district.c - Delete an entire district directory
 */

int remove_district(const char *district, const Args *args)
{
    if (args->role != ROLE_MANAGER)
    {
        fprintf(stderr, "Error: only managers can remove a district\n");
        return -1;
    }

    (void)district;
    fprintf(stderr, "remove_district: not yet implemented\n");
    return 0;
}
