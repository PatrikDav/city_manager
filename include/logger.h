/*
 * logger.h - Operation Logging to logged_district
 *
 * Every action performed on a district is recorded in a plain-text file
 * called logged_district inside the district directory.
 *
 * LOG FORMAT:
 *   One line per operation:
 *   [YYYY-MM-DD HH:MM:SS] role=<role> user=<name> op=<OPERATION>
 *
 *   Example:
 *   [2026-04-16 14:30:00] role=inspector user=John op=LIST
 *   [2026-04-16 14:31:05] role=manager user=Alice op=REMOVE id=3
 */

#ifndef LOGGER_H
#define LOGGER_H

// log_operation - Append one timestamped log entry to logged_district
int log_operation(const char *district, const char *role_str, const char *username, const char *operation);

#endif /* LOGGER_H */
