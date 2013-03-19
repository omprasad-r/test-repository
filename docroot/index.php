<?php

/**
 * @file
 * The PHP page that serves all page requests on a Drupal installation.
 *
 * The routines here dispatch control to the appropriate handler, which then
 * prints the appropriate page.
 *
 * All Drupal code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt.
 */
##GardensExcludeFromExportStart################################################
// When a Gardens site is deleted and a new site is created using the same URL
// (and therefore with the same symlink in the sites directory in the file
// system, but with the symlink pointed to a different target), some
// combination of the PHP realpath cache and the APC cache can result in the
// old site's settings.php and other files still being used when the new site
// is visited. To fix this, clearing out the PHP static cache at the beginning
// of all Drupal page requests seems to be the only reliable solution. This
// means that in Drupal Gardens, we do not have any cross-request caching of
// files in PHP, but still can take advantage of the cache within a single page
// request.
clearstatcache();
##################################################GardensExcludeFromExportEnd##
/**
 * Root directory of Drupal installation.
 */
define('DRUPAL_ROOT', getcwd());

require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
menu_execute_active_handler();
