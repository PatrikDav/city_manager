/*
 * args.h - Command-line args handling for city_manager program
 *
 * This header defines the data types used to represent parsed command-line
 * arguments. The city_manager program is invoked like:
 *
 *   ./city_manager --role <role> --user <name> --<command> <district> [extra args]
 *
 * Examples:
 *   ./city_manager --role inspector --user John --add downtown
 *   ./city_manager --role manager   --user Alice --remove_report downtown 5
 *   ./city_manager --role inspector --user John --filter downtown severity:>=:2
 *
 * The parse_args() function (implemented in args.c) walks through argv[],
 * recognizes each flag, and fills an Args struct with all the parsed values.
 * Then main.c reads the Args struct to decide which operation to perform.
 *
 */

#ifndef ARGS_H
#define ARGS_H

#include "report.h" /* For NAME_LEN constant (used for username/district size) */

/*The two roles in the system. Passed via --role on the command line.
 *
 * HOW ROLES MAP TO FILE PERMISSIONS:
 *   - ROLE_MANAGER  -> treated as the file OWNER  (checks owner bits: rwx------)
 *   - ROLE_INSPECTOR -> treated as the file GROUP (checks group bits: ---rwx---)
 *
 */
typedef enum
{
    ROLE_INSPECTOR, /* Value 0 - city inspector          */
    ROLE_MANAGER    /* Value 1 - city manager            */
} Role;

/* Commands */
typedef enum
{
    CMD_ADD,              /* --add <district>                     : Append a new report       */
    CMD_LIST,             /* --list <district>                    : List all reports           */
    CMD_VIEW,             /* --view <district> <report_id>        : Show one report's details  */
    CMD_REMOVE_REPORT,    /* --remove_report <district> <id>      : Delete a report            */
    CMD_UPDATE_THRESHOLD, /* --update_threshold <district> <val>  : Set severity threshold      */
    CMD_FILTER,           /* --filter <district> <conditions...>  : Filter reports by criteria  */
    CMD_UNKNOWN           /* No valid command was found on the command line                     */
} Command;

/* Arguments Struct */
typedef struct
{
    Role role;                /* Role: inspector or manager */
    char username[NAME_LEN];  /* User: */
    Command command;          /* Command */
    char district[NAME_LEN];  /* The district */
    int report_id;            /* Report id     */
    int threshold_value;      /* Threshold value  */
    int filter_count;         /* Number of filter conditions             */
    char **filter_conditions; /* Array of condition strings  */
} Args;

/* Function Declaration
 *
 * Params:
 *   argc - number of args
 *   argv - array of args
 *   args - pointer to an Args struct with values
 *
 * Returns:
 *    0 on success
 *   -1 on error
 */
int parse_args(int argc, char *argv[], Args *args);

/* help message showing all valid command-line formats */
void print_usage(const char *program_name);

#endif /* ARGS_H */
