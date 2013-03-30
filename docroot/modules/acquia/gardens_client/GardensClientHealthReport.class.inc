<?php

/**
 * Tests whether or not there are xmlrpc credentials.
 */
if (class_exists('GardensHealthReport')) {
  class GardensClientHealthReport extends GardensHealthReport {
    public function checkStatus() {
      if (!function_exists('_acquia_gardens_xmlrpc_creds')) {
        $path	= DRUPAL_ROOT .'/../library';
        @include_once("$path/acquia_gardens_xmlrpc.inc");
      }

      $creds = _acquia_gardens_xmlrpc_creds('gardener');
      $this->addMessage('Each gardens distro expects to have a credential file located at /mnt/gfs/nobackup/gardens_xmlrpc_creds.ini.');

      return empty($creds) ? GARDENS_HEALTH_STATUS_FAIL : GARDENS_HEALTH_STATUS_PASS;
    }
  }
}
