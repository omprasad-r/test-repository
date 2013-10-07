#! /bin/bash

# This script creates symlinks in /docroot/files based on the domains previously
# contained in /docroot/sites.  These new symlinks point to the files
# directories in gluster and are created in this way in order to support any
# legacy urls that may be lingering - for example if someone copypasted an image
# url manually into their node bodies.  The symlinks created by this script work
# in tandem with an .htaccess rewrite rule to be able to support the old urls of
# the form sites/<domain>/files... which we rewrite in .htaccess to
# /files/domain... .
#
# We don't check that symlinks can be written because this script should be the
# first time they've ever been written - the only symlinks created should be
# from this script, as the site user. A quick survey of the /docroot/files dir
# on WMG and SMB shows it should be writable by the site user.
#
# Per the shebang line above, this must only be run under bash, not sh, which
# seems to turn out to be dash.

usage() {
  echo 'Usage: domainsymlinks.sh <sitegroup> <environment> <update environment> [limit]'
}

# Sanity check we have a sitegroup, env and up_env

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]
then
  usage
  exit
fi

# Sanity check this is running as the sitegroup user so we don't create symlinks unalterable by the sitegroup user.

if [ "$1" !=  $USER ]
then
  echo "Looks like your sitegroup ($1) does not match the current user ($USER). I'm sorry, $USER, I'm afraid I can't do that."
  exit 1
fi

SITEGROUP=$1
ENV=$2
UP_ENV=$3

LIMIT=$4

if [ ! -d /mnt/www/html/$SITEGROUP.$ENV ] || [ ! -d /mnt/www/html/$SITEGROUP.$UP_ENV ] || [ ! -d /mnt/gfs/$SITEGROUP.$ENV/sites/g/files ]
then
  echo "Required directories not present. Aborting."
  exit 1
fi

BASE=/mnt/www/html/$SITEGROUP.$ENV/docroot
UP_BASE=/mnt/www/html/$SITEGROUP.$UP_ENV/docroot
FILES_BASE=/mnt/gfs/$SITEGROUP.$ENV/sites/g/files
counter=0
while read DOMAIN_DIR; do
  if [ "$LIMIT" != "" ] && [ "$counter" -ge "$LIMIT" ]
  then
    echo "Aborting because you asked me to limit to $LIMIT symlinks."
    break
  fi

  DOMAIN=$(basename $DOMAIN_DIR);
  LINK=$BASE/files/$DOMAIN;
  UP_LINK=$UP_BASE/files/$DOMAIN;
  SITE_ID=$(basename $(readlink $DOMAIN_DIR));
  DIR=$FILES_BASE/$SITE_ID/f;

  # Create links in the live site dir
  if [ -L $LINK ] && [ $(readlink $LINK) = $DIR ]; then
    echo "Link exists $LINK => $DIR"
  else
    echo "Creating link $LINK => $DIR"
    ln -s $DIR $LINK;
  fi

  # Create links in the up site dir.
  if [ -L $UP_LINK ] && [ $(readlink $UP_LINK) = $DIR ]; then
    echo "Link exists $UP_LINK => $DIR"
  else
    echo "Creating link $UP_LINK => $DIR"
    ln -s $DIR $UP_LINK;
  fi

  let counter+=1

done < <( find $BASE/sites/* -maxdepth 0 -type l )
