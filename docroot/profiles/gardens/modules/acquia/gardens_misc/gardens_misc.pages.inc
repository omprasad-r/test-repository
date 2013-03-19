<?php

/**
 * Page callback to report site statistics, and run cron if appropriate.
 */
function gardens_misc_report_site_statistics() {
  if (!empty($_GET['force'])) {
    // Force the site statistics to be sent even if they were sent
    // less than 24 hours ago.  This is important for testing
    // purposes, but generally not a great idea for production due to
    // the increased load on the system.
    variable_del('gardens_stats_time');
  }

  // If the site is in maintenance mode, simply report statistics, without
  // running the rest of cron.
  if (variable_get('maintenance_mode', 0)) {
    gardens_client_cron();
  }
  else {
    // Run the gardens_client_cron first just in case a cron failure
    // gets in the way of sending the site statistics back to gardener.
    gardens_client_cron();
    drupal_cron_run();
  }
}
