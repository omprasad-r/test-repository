#!/bin/bash
# header
# basetheme, version, adv css line count, theme path
# you need to drag getThemeInfo.php along with this script
# this should be run only on bal-42 so that it is run on the local brick
for tangle in tangle001 tangle002 tangle004 tangle005 tangle006 tangle007 tangle008
do 
  find  /mnt/brick5/${tangle}/gardens-sites/ -type d -name 'acq*' | grep -v session | xargs -I %THEME ./getThemeInfo.php %THEME | tee ./${tangle}themeversion.csv
done
