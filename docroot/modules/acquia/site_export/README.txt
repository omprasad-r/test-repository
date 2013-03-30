To give you better control over your site-specific files, Drupal
Gardens provides the get_files.sh script so you can retrieve your
site-specific files during the export process.

To complete your export and retrieve your files, perform the following steps:
 
 1.  Install the curl utility.
 2.  From the /docroot directory, run the get_files.sh script.

$ cd /<path to drupal docroot>
$ ./get_files.sh
    
Note: You must run the script from the /docroot directory so the
downloaded files go to the correct location.
