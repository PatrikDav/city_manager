/*
 * district.h - District Directory Initialization
 *
 * A "district" in city_manager is a directory under data/ that holds three
 * files for that area of the city:
 *
 *   data/<district_name>/
 *   ├── reports.dat       Binary file of fixed-size Report records
 *   ├── district.cfg      Plain text config (severity threshold)
 *   └── logged_district   Plain text operation log
 */

#ifndef DISTRICT_H
#define DISTRICT_H

// ensure_district - Create the district directory structure if it doesn't exist
int ensure_district(const char *name);

#endif /* DISTRICT_H */
