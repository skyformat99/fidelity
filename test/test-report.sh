#!/bin/bash -e

if [[ $1 = @(-h|--help) ]] || [ -z $1 ]; then
  echo "Summarize gtest reports."
  echo "Usage: ${0##*/} <test-report> [<more-test-reports>]"
  echo ""; exit 0
fi

TESTLOG=${TESTLOG:-/dev/null}
echo

for TEST in $@; do
  SUITE=${TEST%.xml}
  REPORT=$SUITE.xml

  if [ ! -f "$REPORT" ]; then
    >&2 echo "==> ERROR: Report file $REPORT not found."
    PASS=false
    continue
  fi

  RUNS=$(xmlstarlet sel -t -m "testsuites" -v "@tests"    < $REPORT)
  FAIL=$(xmlstarlet sel -t -m "testsuites" -v "@failures" < $REPORT)
  SKIP=$(xmlstarlet sel -t -m "testsuites" -v "@disabled" < $REPORT)
  ERRS=$(xmlstarlet sel -t -m "testsuites" -v "@errors"   < $REPORT)

  if [ $FAIL -ne 0 ] || [ $ERRS -ne 0 ]; then
    PASS=false
  fi

  FAILED_CASES=$(
    xmlstarlet sel -t -m "//testcase[failure]" -v "../@name" -o "_" -v "@name" -n < $REPORT || true
  )
  for failed in $FAILED_CASES; do
    FAILED_TESTS+=" ${SUITE##*/}:$failed"
  done

  {
    echo -n "${SUITE##*/}, $RUNS tests"
    [ $FAIL -ne 0 ] && echo -n ", $FAIL failed"
    [ $SKIP -ne 0 ] && echo -n ", $SKIP disabled"
    [ $ERRS -ne 0 ] && echo -n ", $ERRS errors"
    echo
  } | tee -a $TESTLOG

done

for failed in $FAILED_TESTS; do
  echo "failure: ${failed//:/ }" | tee -a $TESTLOG
done
echo

if ! $PASS; then
  exit 1
fi
