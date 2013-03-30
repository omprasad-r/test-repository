Deploying a new customer on Drupal Gardens
==========================================
This document is specific to deploying Gardens 2.0 using the Acquia Hosting platform.

Manual steps
------------
1. XMLRPC Configuration
    1. Set xmlrpc basic auth credentials in the gardener (Method TBD)
    1. Deploy xmlrpc credential file to /mnt/gfs/nobackup/gardens_xmlrpc_creds.ini
1. Cloud API Configuration
    1. Create a network subscription for each tangle user
    1. Deploy cloudapi credential file to /mnt/gfs/nobackup/cloudapi.ini

