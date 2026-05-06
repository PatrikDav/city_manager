/*
 * monitor_notify.h - Notify the monitor_reports process of a new report
 *
 * city_manager calls notify_monitor() right after a successful add_report.
 * It reads data/.monitor_pid, parses the PID, and sends SIGUSR1.
 *
 * The caller passes a buffer that this function fills with a log-ready
 * message: "MONITOR_NOTIFIED pid=<n>" on success, or
 * "MONITOR_NOT_INFORMED reason=<...>" on any failure path. The spec
 * requires the log to be explicit when the monitor could not be reached,
 * so the caller logs the buffer either way.
 */

#ifndef MONITOR_NOTIFY_H
#define MONITOR_NOTIFY_H

#include <stddef.h>

// notify_monitor - send SIGUSR1 to the monitor process.
// Fills out_msg with a log-ready status string in all cases.
// Returns 0 on success, -1 on any failure.
int notify_monitor(char *out_msg, size_t out_size);

#endif /* MONITOR_NOTIFY_H */
