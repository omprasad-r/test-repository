#!/bin/sh

if `command -v xvfb-run >/dev/null`; then
  CUCUMBER="xvfb-run -a cucumber"
else
  CUCUMBER="bundle exec cucumber"
fi

# Loop while installing gems to avoid build fails due to connection problems
for i in `seq 10`; do
  bundle install --binstubs --path bundler_gems/
  if [ $? -eq 0 ]; then
    break;
  fi
done

export SUT_URL="http://${SUBDOMAIN}.gardensqa.acquia-sites.com"
#for the gems
export PATH=./bin:$PATH

case $1 in
  "all") TAGS="--tags ~@wip"
    ;;
  "smoke") TAGS="--tags @smoke"
    ;;
  "debug") FEATURE="$2"
    ;;
  *) TAGS="--tags ~@wip"
    ;;
esac

$CUCUMBER --require features $TAGS --format junit --out junit_reports --format html --out test_results.html --format pretty $FEATURE
