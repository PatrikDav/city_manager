#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "report.h"
#include "district.h"
#include "logger.h"
#include "report_io.h"
#include "filter.h"
#include "remove_district.h"

/*
 * main.c - Entry Point for city_manager
 */

int main(int argc, char *argv[])
{
    Args args;

    if (parse_args(argc, argv, &args) != 0)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (args.command != CMD_REMOVE_DISTRICT &&
        ensure_district(args.district) != 0)
        return EXIT_FAILURE;

    switch (args.command)
    {
    case CMD_ADD:
        add_report(args.district, &args);
        break;

    case CMD_LIST:
        list_reports(args.district, &args);
        break;

    case CMD_VIEW:
        view_report(args.district, &args);
        break;

    case CMD_REMOVE_REPORT:
        remove_report(args.district, &args);
        break;

    case CMD_UPDATE_THRESHOLD:
        update_threshold(args.district, &args);
        break;

    case CMD_FILTER:
        filter_reports(args.district, &args);
        break;

    case CMD_REMOVE_DISTRICT:
        remove_district(args.district, &args);
        break;

    case CMD_UNKNOWN:
        fprintf(stderr, "Error: unknown command\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
