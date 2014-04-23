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
   * /mnt/www/html/[site].[env]/docroot will have a credential file at
   * /mnt/gfs/[site].[env]/nobackup/sf_shared_creds.ini, using normal INI format:
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
    $ini_file = sprintf('/mnt/gfs/%s/nobackup/sf_shared_creds.ini', $site_name);

    // After everyone has 2.09, this can be removed.
    if (!file_exists($ini_file)) {
      $ini_file = sprintf('/mnt/gfs/%s/nobackup/gardens_xmlrpc_creds.ini', $site_name);
    }

    $acsf_shared_creds = parse_ini_file($ini_file, TRUE);

    if (empty($acsf_shared_creds['gardener'])) {
      throw new AcsfConfigMissingCredsException(sprintf('Shared credential file not found in /mnt/gfs/%s/nobackup/', $site_name));
    }

    $this->url = $acsf_shared_creds['gardener']['url'];
    $this->username = $acsf_shared_creds['gardener']['username'];
    $this->password = $acsf_shared_creds['gardener']['password'];
    if (isset($acsf_shared_creds['gardener']['url_suffix'])) {
      $this->url_suffix = $acsf_shared_creds['gardener']['url_suffix'];
    }
    if (isset($acsf_shared_creds['gardener']['source_url'])) {
      $this->source_url = $acsf_shared_creds['gardener']['source_url'];
    }
  }

}
