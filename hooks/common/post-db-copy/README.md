Acquia Cloud Site Factory/Acquia Cloud Hooks
============================================
The Site Factory uses Acquia Cloud for hosting, and as such as access to all Acquia Cloud APIs. Site Factory customers may incorporate their own cloud hooks with the caveat that they cannot run before the "acquia_required_scrub.php" file. The files are executed in lexicographical order, so any custom scripts must be listed _after_ that file.

Please see the Site Factory repository (/hooks/README.md) for more information about cloud hooks.
