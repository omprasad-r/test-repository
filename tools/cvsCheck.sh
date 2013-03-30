#!/bin/bash

i=$1;
echo "Checking $i";
if [ ! -d "$i/CVS" ]; then
    echo "Not in CVS"
    exit
  fi
  cd $i;
  
  ONEFILE=`ls *.* | head -n1`
  
  if [ -f 'CVS/TAG' ]; then
    cat CVS/TAG
  else
    cvs stat "$ONEFILE" | grep "Needs";
  fi
  echo "Available tags:"
  
   cvs status -v "$ONEFILE" | grep DRUPAL-7- ;
  cd ../;
  
  

#for i in `ls -d */ | grep -v "CVS"`;
# do
#  
#done