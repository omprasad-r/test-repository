<?php

/**
 * Use this script to test that a gardens site can connect to the gardener. It
 * should be executed from the site's directory or you should use a drush alias.
 *
 * Usage: drush scr /path/to/this/script
 */
require_once "acquia_gardens_xmlrpc.inc";
$creds = _acquia_gardens_xmlrpc_creds('gardener');
print "Credentials:";
var_dump($creds);
$site = variable_get('gardens_misc_standard_domain', 'unknown');
$out = gardens_client_call_gardener('test.gardener', array($site));
print "Response:";
var_dump($out);
