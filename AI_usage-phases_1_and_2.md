# AI Usage Documentation — city_manager

This file documents every function that was generated or significantly shaped with AI assistance,
as required by the Phase 1 assignment specification.

---

## Functions: `parse_condition` and `match_condition`
**File:** `src/filter.c`

---

### Prompt given to the AI

> I am writing a C filter system for a city infrastructure reporting tool.
> Reports are stored as fixed-size binary structs with fields: report_id (int),
> inspector_name (char[64]), category (char[32]), severity (int), timestamp (time_t),
> gps_lat/gps_lon (float), description (char[256]).
>
> I need two functions:
>
> 1. `parse_condition(const char *input, char *field, char *op, char *value)`
>    - Input is a string in the format "field:operator:value", e.g. "severity:>=:2"
>    - It should split the string into three parts: field, operator, and value
>    - Return 0 on success, -1 if the string is malformed (missing colons)
>
> 2. `match_condition(const Report *r, const char *field, const char *op, const char *value)`
>    - Given a Report struct and a parsed condition, return 1 if the report satisfies it
>    - Numeric fields (severity, timestamp) should use integer comparison
>    - String fields (category, inspector_name) should use strcmp
>    - Supported operators: ==, !=, <, <=, >, >=

---

### What the AI generated

The AI produced both functions. The core logic matched what I needed:

**`parse_condition`:** Used `strchr()` to find the first and second colons, then
`strncpy()` to copy each segment into the output buffers. Correctly handles the
case where either colon is missing (returns -1).

**`match_condition`:** Used two internal helper functions — `compare_int()` for
integer fields and `compare_str()` for string fields. Each helper takes an operator
string and applies it via a chain of `strcmp(op, "==") == 0` checks.
`match_condition` itself dispatches on the field name using `strcmp`.

---

### Changes made after generation

- Added `compare_int` and `compare_str` as `static` helpers (the AI had them
  as non-static; making them static limits their scope to this translation unit).
- Renamed the inspector field dispatch from `"inspector_name"` to `"inspector"`
  to match the shorter command-line convention used in the rest of the project
  (e.g. `inspector:==:Maria` instead of `inspector_name:==:Maria`).
- Added a `fprintf(stderr, ...)` warning for unknown field names instead of
  silently returning 0 — this makes debugging bad filter conditions easier.
- Replaced `strncat`-based value copy (AI used it incorrectly with a wrong size)
  with a direct `strncpy(value, second_colon + 1, 255); value[255] = '\0'`.
- Added the comment block at the top of the file explaining the AI involvement
  and pointing to this document.

---

### What I learned

- `strchr()` returns a pointer into the original string, so pointer arithmetic
  (`second_colon - (first_colon + 1)`) gives the exact length of the operator
  substring without needing a second pass.
- Using `static` for file-local helpers is good practice in C — it prevents
  name collisions if another translation unit defines a function with the same name.
- `strncat` is easy to misuse (the size argument is the *remaining space*, not
  the total buffer size). `strncpy` + explicit null-terminator is safer here.

---

## Phase 2

No AI tools were used in Phase 2. All signal handling, fork/exec, monitor, and notification logic was written manually.
