This directory is used for building Gardens core and contrib codebases via a Drush makefile.

The standard makefile is located at ../gardens.make, and there is a script for working with
it at ../tools/update-core-and-contrib.php.

Typically you will wind up creating a temporary subdirectory within this directory (e.g.,
called 'gardens') in which the codebase is built, then rysnc that directory to ../docroot
where the actual Gardens code lives in SVN.
