<?php

class AcsfLog {

  /**
   * Logs the specified message to the Site Factory over XML-RPC.
   *
   * @param array $record
   *   The parameters to send to the Site Factory.
   */
  public function log($record) {
    if (!$this->enabled()) {
      return;
    }

    try {
      $message = new AcsfMessageXmlRpc('GET', 'acquia.sf.log.send', array($record));
      $message->send();
      return $message->getResponseBody();
    }
    catch (Exception $e) {
      // Swallow exceptions.
    }
  }

  /**
   * Determines whether logging is enabled or blocked globally.
   */
  public function enabled() {
    $site = $_ENV['AH_SITE_GROUP'];
    $env = $_ENV['AH_SITE_ENVIRONMENT'];
    return !file_exists(sprintf('/mnt/gfs/%s.%s/files-private/sf-log-block', $site, $env));
  }

}

