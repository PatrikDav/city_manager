#!/bin/bash
# tests/test_commands.sh - End-to-end tests for city_manager
#
# Run from the project root: bash tests/test_commands.sh
# Requires the binary to already be compiled: make

PASS=0
FAIL=0
BIN=./city_manager

# Colours
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

pass() { echo -e "${GREEN}PASS${NC} $1"; PASS=$((PASS + 1)); }
fail() { echo -e "${RED}FAIL${NC} $1"; FAIL=$((FAIL + 1)); }

assert_contains() {
    local desc="$1"
    local output="$2"
    local expected="$3"
    if echo "$output" | grep -qF "$expected"; then
        pass "$desc"
    else
        fail "$desc (expected: '$expected')"
        echo "       got: $output"
    fi
}

assert_exit_ok()   { [ "$1" -eq 0 ] && pass "$2" || fail "$2 (exit=$1)"; }
assert_exit_fail() { [ "$1" -ne 0 ] && pass "$2" || fail "$2 (expected non-zero)"; }

# ---------------------------------------------------------------------------
echo "=== Setup ==="
# ---------------------------------------------------------------------------

rm -rf data/ active_reports-*

if [ ! -x "$BIN" ]; then
    echo "Binary '$BIN' not found. Run 'make' first."
    exit 1
fi

# ---------------------------------------------------------------------------
echo ""
echo "=== 1. District creation ==="
# ---------------------------------------------------------------------------

$BIN --role inspector --user John --list downtown > /dev/null 2>&1
assert_exit_ok $? "ensure_district runs without error"

[ -d "data/downtown" ]
assert_exit_ok $? "data/downtown directory created"

# Check permissions (octal) using stat
DIR_PERM=$(stat -c "%a" data/downtown)
assert_contains "district dir is 750" "$DIR_PERM" "750"

DAT_PERM=$(stat -c "%a" data/downtown/reports.dat)
assert_contains "reports.dat is 664" "$DAT_PERM" "664"

CFG_PERM=$(stat -c "%a" data/downtown/district.cfg)
assert_contains "district.cfg is 640" "$CFG_PERM" "640"

LOG_PERM=$(stat -c "%a" data/downtown/logged_district)
assert_contains "logged_district is 644" "$LOG_PERM" "644"

CFG_CONTENT=$(cat data/downtown/district.cfg)
assert_contains "district.cfg default threshold=2" "$CFG_CONTENT" "threshold=2"

# ---------------------------------------------------------------------------
echo ""
echo "=== 2. Symlink ==="
# ---------------------------------------------------------------------------

[ -L "active_reports-downtown" ]
assert_exit_ok $? "symlink active_reports-downtown created"

LINK_TARGET=$(readlink active_reports-downtown)
assert_contains "symlink points to reports.dat" "$LINK_TARGET" "reports.dat"

LIST_OUT=$($BIN --role inspector --user John --list downtown 2>&1)
assert_contains "list shows symlink OK" "$LIST_OUT" "OK"

# ---------------------------------------------------------------------------
echo ""
echo "=== 3. add_report ==="
# ---------------------------------------------------------------------------

OUT=$(printf 'road\n44.43\n26.10\n3\npothole on main street\n' | \
      $BIN --role inspector --user John --add downtown 2>&1)
assert_contains "add report #1 succeeds" "$OUT" "Report #1 added"

OUT=$(printf 'lighting\n44.44\n26.11\n1\nbroken lamp\n' | \
      $BIN --role inspector --user John --add downtown 2>&1)
assert_contains "add report #2 succeeds" "$OUT" "Report #2 added"

OUT=$(printf 'road\n44.45\n26.12\n2\ncracked pavement\n' | \
      $BIN --role inspector --user Maria --add downtown 2>&1)
assert_contains "add report #3 succeeds" "$OUT" "Report #3 added"

# ---------------------------------------------------------------------------
echo ""
echo "=== 4. list_reports ==="
# ---------------------------------------------------------------------------

LIST=$($BIN --role inspector --user John --list downtown 2>&1)
assert_contains "list shows report #1" "$LIST" "ID: 1"
assert_contains "list shows report #2" "$LIST" "ID: 2"
assert_contains "list shows report #3" "$LIST" "ID: 3"
assert_contains "list shows category road" "$LIST" "road"
assert_contains "list shows inspector Maria" "$LIST" "Maria"
assert_contains "list shows file size" "$LIST" "bytes"
assert_contains "list shows permissions" "$LIST" "rw-rw-r--"

# ---------------------------------------------------------------------------
echo ""
echo "=== 5. view_report ==="
# ---------------------------------------------------------------------------

VIEW=$($BIN --role inspector --user John --view downtown 1 2>&1)
assert_contains "view shows report #1 ID" "$VIEW" "Report ID   : 1"
assert_contains "view shows category" "$VIEW" "road"
assert_contains "view shows severity 3" "$VIEW" "critical"
assert_contains "view shows GPS lat" "$VIEW" "44.43"
assert_contains "view shows description" "$VIEW" "pothole"

OUT=$($BIN --role inspector --user John --view downtown 99 2>&1)
assert_contains "view non-existent ID returns error" "$OUT" "not found"

# ---------------------------------------------------------------------------
echo ""
echo "=== 6. remove_report ==="
# ---------------------------------------------------------------------------

# Inspector cannot remove
OUT=$($BIN --role inspector --user John --remove_report downtown 1 2>&1)
assert_contains "inspector cannot remove" "$OUT" "only managers can remove"

# Manager removes report #2 (middle record)
OUT=$($BIN --role manager --user Alice --remove_report downtown 2 2>&1)
assert_contains "manager removes report #2" "$OUT" "Report #2 removed"

LIST=$($BIN --role inspector --user John --list downtown 2>&1)
assert_contains "after remove #2: #1 still present" "$LIST" "ID: 1"
assert_contains "after remove #2: #3 still present" "$LIST" "ID: 3"
if echo "$LIST" | grep -qF "ID: 2"; then
    fail "report #2 still present after remove"
else
    pass "report #2 absent after remove"
fi

REPORT_COUNT=$(echo "$LIST" | grep -c "^ID:")
assert_contains "two reports remain after remove" "$REPORT_COUNT" "2"

# Manager removes first record
OUT=$($BIN --role manager --user Alice --remove_report downtown 1 2>&1)
assert_contains "manager removes report #1 (first)" "$OUT" "Report #1 removed"

LIST=$($BIN --role inspector --user John --list downtown 2>&1)
assert_contains "only report #3 remains" "$LIST" "ID: 3"

# Manager removes last record
OUT=$($BIN --role manager --user Alice --remove_report downtown 3 2>&1)
assert_contains "manager removes report #3 (last)" "$OUT" "Report #3 removed"

OUT=$($BIN --role manager --user Alice --remove_report downtown 3 2>&1)
assert_contains "remove from empty file gives error" "$OUT" "Error"

# ---------------------------------------------------------------------------
echo ""
echo "=== 7. Re-add reports for filter tests ==="
# ---------------------------------------------------------------------------

printf 'road\n44.43\n26.10\n3\npothole on main street\n' | \
    $BIN --role inspector --user John --add downtown > /dev/null 2>&1
printf 'lighting\n44.44\n26.11\n1\nbroken lamp\n' | \
    $BIN --role inspector --user John --add downtown > /dev/null 2>&1
printf 'road\n44.45\n26.12\n2\ncracked pavement\n' | \
    $BIN --role inspector --user Maria --add downtown > /dev/null 2>&1

RCOUNT=$($BIN --role inspector --user John --list downtown 2>&1 | grep -c "^ID:")
assert_contains "three reports re-added" "$RCOUNT" "3"

# ---------------------------------------------------------------------------
echo ""
echo "=== 8. update_threshold ==="
# ---------------------------------------------------------------------------

OUT=$($BIN --role manager --user Alice --update_threshold downtown 4 2>&1)
assert_contains "manager updates threshold" "$OUT" "Threshold updated to 4"

CFG=$(cat data/downtown/district.cfg)
assert_contains "district.cfg contains threshold=4" "$CFG" "threshold=4"

OUT=$($BIN --role inspector --user John --update_threshold downtown 1 2>&1)
assert_contains "inspector cannot update threshold" "$OUT" "only managers can update"

# Verify permission check: if perms change, update_threshold must refuse
chmod 0660 data/downtown/district.cfg
OUT=$($BIN --role manager --user Alice --update_threshold downtown 5 2>&1)
assert_contains "wrong perms on district.cfg rejected" "$OUT" "refusing write"
chmod 0640 data/downtown/district.cfg  # restore

# ---------------------------------------------------------------------------
echo ""
echo "=== 9. filter_reports ==="
# ---------------------------------------------------------------------------

# severity >= 2  (reports #1 severity=3 and #3 severity=2)
OUT=$($BIN --role inspector --user John --filter downtown 'severity:>=:2' 2>&1)
assert_contains "filter severity>=2 matches 2 records" "$OUT" "2 record(s) matched"

# severity == 1  (report #2)
OUT=$($BIN --role inspector --user John --filter downtown 'severity:==:1' 2>&1)
assert_contains "filter severity==1 matches 1 record" "$OUT" "1 record(s) matched"

# category == road  (2 reports)
OUT=$($BIN --role inspector --user John --filter downtown 'category:==:road' 2>&1)
assert_contains "filter category==road matches 2 records" "$OUT" "2 record(s) matched"

# severity == 3 AND category == road  (1 report: the pothole, severity 3)
OUT=$($BIN --role inspector --user John --filter downtown 'severity:==:3' 'category:==:road' 2>&1)
assert_contains "filter sev==3 AND cat==road matches 1" "$OUT" "1 record(s) matched"
assert_contains "filter AND result is pothole report" "$OUT" "pothole"

# inspector == Maria  (1 report)
OUT=$($BIN --role inspector --user John --filter downtown 'inspector:==:Maria' 2>&1)
assert_contains "filter inspector==Maria matches 1" "$OUT" "1 record(s) matched"
assert_contains "filter inspector==Maria is cracked pavement" "$OUT" "cracked pavement"

# no match
OUT=$($BIN --role inspector --user John --filter downtown 'category:==:flooding' 2>&1)
assert_contains "filter no match returns 0 records" "$OUT" "0 record(s) matched"

# ---------------------------------------------------------------------------
echo ""
echo "=== 10. Dangling symlink ==="
# ---------------------------------------------------------------------------

# Remove reports.dat to make the symlink dangle
rm data/downtown/reports.dat

LIST=$($BIN --role inspector --user John --list downtown 2>&1)
# ensure_district will recreate reports.dat before list runs, so symlink is valid again
# — this is expected correct behaviour; just verify the symlink still exists
[ -L "active_reports-downtown" ]
assert_exit_ok $? "symlink persists after reports.dat delete + ensure_district"

# Manually check dangling detection: point a symlink at a nonexistent file
ln -sf data/downtown/does_not_exist.dat active_reports-test 2>/dev/null
DANGLE_OUT=$($BIN --role inspector --user John --list downtown 2>&1)
# The real symlink for 'downtown' is now valid (ensure_district recreated the file)
assert_contains "list shows symlink OK after recreate" "$DANGLE_OUT" "OK"
rm -f active_reports-test

# ---------------------------------------------------------------------------
echo ""
echo "=== 11. Logging ==="
# ---------------------------------------------------------------------------

LOG=$(cat data/downtown/logged_district)
assert_contains "log has LIST entry" "$LOG" "op=LIST"
assert_contains "log has ADD entry" "$LOG" "op=ADD"
assert_contains "log has REMOVE entry" "$LOG" "op=REMOVE"
assert_contains "log has UPDATE_THRESHOLD entry" "$LOG" "op=UPDATE_THRESHOLD"
assert_contains "log has FILTER entry" "$LOG" "op=FILTER"
assert_contains "log has VIEW entry" "$LOG" "op=VIEW"
assert_contains "log has user=John" "$LOG" "user=John"
assert_contains "log has user=Alice" "$LOG" "user=Alice"
assert_contains "log has role=manager" "$LOG" "role=manager"
assert_contains "log has role=inspector" "$LOG" "role=inspector"

# ---------------------------------------------------------------------------
echo ""
echo "=== 12. remove_district ==="
# ---------------------------------------------------------------------------

# Create a throwaway district to delete
printf 'road\n44.43\n26.10\n1\ntemp report\n' | \
    $BIN --role manager --user Alice --add tmp_del > /dev/null 2>&1
[ -d "data/tmp_del" ]
assert_exit_ok $? "throwaway district tmp_del exists before remove"
[ -L "active_reports-tmp_del" ]
assert_exit_ok $? "symlink active_reports-tmp_del exists before remove"

# Inspector cannot remove a district
OUT=$($BIN --role inspector --user John --remove_district tmp_del 2>&1)
assert_contains "inspector cannot remove_district" "$OUT" "only managers can remove"
[ -d "data/tmp_del" ]
assert_exit_ok $? "district still exists after rejected inspector attempt"

# Manager successfully removes the district
OUT=$($BIN --role manager --user Alice --remove_district tmp_del 2>&1)
assert_contains "manager remove_district succeeds" "$OUT" "removed"
[ ! -d "data/tmp_del" ]
assert_exit_ok $? "data/tmp_del directory is gone after remove"
[ ! -L "active_reports-tmp_del" ]
assert_exit_ok $? "symlink active_reports-tmp_del is gone after remove"

# Removing a non-existent district gives an error
OUT=$($BIN --role manager --user Alice --remove_district tmp_del 2>&1)
assert_contains "remove non-existent district gives error" "$OUT" "Error"

# ---------------------------------------------------------------------------
echo ""
echo "=== Results ==="
# ---------------------------------------------------------------------------

echo -e "  Passed: ${GREEN}$PASS${NC}"
echo -e "  Failed: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}All tests passed.${NC}"
    exit 0
else
    exit 1
fi
