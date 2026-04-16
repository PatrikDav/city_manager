/*
 * report.h - Report Data Structure
 *
 * This header defines the core data structure for the city_manager program:
 * the Report struct. Every infrastructure issue filed by an inspector is
 * stored as a fixed-size Report record in a binary file (reports.dat).
 *
 */

#ifndef REPORT_H
#define REPORT_H

#include <time.h> /* For timestamps */

/* Size Constants
 * These define the maximum length (in bytes) for each string field with '\0' included.
 */

#define NAME_LEN 64     /* Max length for inspector name*/
#define CATEGORY_LEN 32 /* Max length for issue category*/
#define DESC_LEN 256    /* Max length for description text*/

/*Report Struct
 *
 * MEMORY LAYOUT:
 *   report_id       :   4 bytes (int)
 *   inspector_name  :  64 bytes (char array)
 *   gps_lat         :   4 bytes (float)
 *   gps_lon         :   4 bytes (float)
 *   category        :  32 bytes (char array)
 *   severity        :   4 bytes (int)
 *   timestamp       :   8 bytes (time_t on 64-bit = long = 8 bytes)
 *   description     : 256 bytes (char array)
 *
 */

typedef struct
{
    int report_id;                 /* Unique identifier*/
    char inspector_name[NAME_LEN]; /* Name of the inspector*/
    float gps_lat;                 /* Latitude*/
    float gps_lon;                 /* Longitude*/
    char category[CATEGORY_LEN];   /* Issue type*/
    int severity;                  /* 1=minor, 2=moderate, 3=critical*/
    time_t timestamp;              /* When the report was created*/
    char description[DESC_LEN];    /* Text description*/
} Report;

#endif /* REPORT_H */
