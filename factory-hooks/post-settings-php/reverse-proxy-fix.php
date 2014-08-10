<?php

/**
 * @file
 * Post-settings-php hook to temporarily fix the reverse proxy array.
 *
 * DG-12396 - The reverse_proxies variables are coming from the settings.inc
 * files which is being created by Hosting. Due to the recent bal-2 relaunching
 * on Warner, the reverse proxies setting is outdated because Site Factory
 * avoids rewriting these settings.inc files. While we figure out how to handle
 * these situation we will have to add the right IP to the config.
 */

if (!in_array('10.145.221.72', $GLOBALS['conf']['reverse_proxies'])) {
  $GLOBALS['conf']['reverse_proxies'][] = '10.145.221.72';
}
