<?php

/**
 * @file
 * Creates a config object using our custom INI file.
 */

class AcsfConfigDefault extends AcsfConfig {

  /**
   * Implements AcsfConfig::loadConfig().
   *
   * The cred file location will match the directory structure of an AH site:
   * /mnt/www/html/[site]/docroot will have a credential file at
   * /mnt/gfs/[site]/nobackup/gardens_xmlrpc_creds.ini, using normal INI format:
   *
   * [gardener]
   * url = "http://gardener.[stage].acquia-sites.com"
   * username = "acquiagardensrpc"
   * password = "[password]"
   * url_suffix = "[stage].acquia-sites.com"
   *
   * @throws AcsfConfigMissingCredsException
   */
  protected function loadConfig() {
    $site_name = sprintf('%s.%s', $this->ah_site, $this->ah_env);
    $gardens_xmlrpc_creds = parse_ini_file("/mnt/gfs/$site_name/nobackup/gardens_xmlrpc_creds.ini", TRUE);

    if (empty($gardens_xmlrpc_creds['gardener'])) {
      throw new AcsfConfigMissingCredsException("XMLRPC credential file not found in at /mnt/gfs/$site_name/nobackup/gardens_xmlrpc_creds.ini");
    }

    $this->url = $gardens_xmlrpc_creds['gardener']['url'];
    $this->username = $gardens_xmlrpc_creds['gardener']['username'];
    $this->password = $gardens_xmlrpc_creds['gardener']['password'];
    if (isset($gardens_xmlrpc_creds['gardener']['url_suffix'])) {
      $this->url_suffix = $gardens_xmlrpc_creds['gardener']['url_suffix'];
    }
    if (isset($gardens_xmlrpc_creds['gardener']['source_url'])) {
      $this->source_url = $gardens_xmlrpc_creds['gardener']['source_url'];
    }
  }

}
