<?php

/**
 * @file
 * Pre-settings-php hook to increase the memory conditionally on few admin pages.
 * This hook has been added to fix the tokens not loading across the sites as per WMG-562
 *
 */
 
 if (
    (strpos($_GET['q'], 'admin') === 0) || (strpos($_GET['q'], 'node/add') === 0) ||
    || (strpos($_GET['q'], 'node/') === 0 && preg_match('/^node\/[\d]+\/edit/', $_GET['q']) === 1)
  ) {
  ini_set('memory_limit', '512M');
}