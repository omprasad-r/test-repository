#!/bin/sh

# Script executes syntax check with cucumber -di and stores result in xls file.

if [ -z "$FIX_VERSION" ]; then
 export FIX_VERSION="Default"
fi

mkdir -p ./junit_reports
mkdir -p ./artifacts
mkdir -p ./artifacts/junit_reports
mkdir -p ./artifacts/screenshots

#remove any existing screenshots prior to run
rm ./artifacts/*.png 2> /dev/null
rm ./artifacts/*.html 2> /dev/null

# make bundle happy
bundle install --path vendor/bundle/
bundle update

bundle exec cucumber -di --format Acquia::Formatter::XLS --out ./artifacts/${FIX_VERSION}SiteFactorySitesTCM.xlsx