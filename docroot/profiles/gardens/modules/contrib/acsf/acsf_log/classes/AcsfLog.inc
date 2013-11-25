<?php

class AcsfLog {

  /**
   * Logs the specified message.
   */
  public function log($severity, $message, $nid = 0) {
    // @todo this is totally temporary, once we get the sf_message endpoint for logs
    // we'll send it there.
    $file = fopen(sprintf('%s/acsf.log', file_directory_temp()), 'a');
    fwrite($file, sprintf('%s - severity: %s - nid: %s - message: %s', format_date(time()), $severity, $nid, $message));
    fclose($file);
  }

}
