<?php

/**
 * @file
 * The AcsfConfig is a simple interface to define how our message configuration should work.
 */

class AcsfConfigIncompleteException extends Exception {}

abstract class AcsfConfig {

  // The URL of the remote service.
  protected $url;

  // The username of the remote service.
  protected $username;

  // The password of the remote service.
  protected $password;

  // An optional Acquia Hosting sitegroup.
  protected $ah_site;

  // An optional Acquia Hosting environment.
  protected $ah_env;

  /**
   * Constructor.
   *
   * @param String $ah_site
   *   (Optional) Acquia Hosting sitegroup.
   * @param String $ah_env
   *   (Optional) Acquia Hosting environment.
   *
   * @throws AcsfConfigIncompleteException
   */
  public function __construct($ah_site = NULL, $ah_env = NULL) {
    // If none specified, pick the site group and environment from $_ENV.
    if (empty($ah_site)) {
      $ah_site = $_ENV['AH_SITE_GROUP'];
    }
    if (empty($ah_env)) {
      $ah_env = $_ENV['AH_SITE_ENVIRONMENT'];
    }

    $this->ah_site = $ah_site;
    $this->ah_env = $ah_env;
    $this->loadConfig();

    // Require the loadConfig implementation to set required values.
    foreach (array('url', 'username', 'password') as $key) {
      if (empty($this->{$key})) {
        throw new AcsfConfigIncompleteException(sprintf('The ACSF configuration was incomplete, no value was found for %s.', $key));
      }
    }
  }

  /**
   * Retrieves the config username.
   *
   * @return String.
   */
  public function getUsername() {
    return $this->username;
  }

  /**
   * Retrieves the config password.
   *
   * @return String.
   */
  public function getPassword() {
    return $this->password;
  }

  /**
   * Retrieves the config URL.
   *
   * @return String.
   */
  public function getURL() {
    return $this->url;
  }

  /**
   * Loads the configuration.
   *
   * Client code MUST populate the url, username and password properties.
   */
  abstract protected function loadConfig();

}
