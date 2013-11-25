<?php

/**
 * @file
 * Populates the AcsfMessage config using our custom INI file.
 */

class AcsfConfigDefault extends AcsfConfig {

  /**
   * Implements AcsfConfig::loadConfig().
   */
  protected function loadConfig() {
    $site_name = sprintf('%s.%s', $this->ah_site, $this->ah_env);
    $gardens_xmlrpc_creds = parse_ini_file("/mnt/gfs/$site_name/nobackup/gardens_xmlrpc_creds.ini", TRUE);
  
    if (empty($gardens_xmlrpc_creds['gardener'])) {
      throw new AcsfMessageMissingCredsException("XMLRPC credential file not found in at /mnt/gfs/$site_name/nobackup/gardens_xmlrpc_creds.ini");
    }
  
    $this->url = $gardens_xmlrpc_creds['gardener']['url'];
    $this->username = $gardens_xmlrpc_creds['gardener']['username'];
    $this->password = $gardens_xmlrpc_creds['gardener']['password'];
  }

}
