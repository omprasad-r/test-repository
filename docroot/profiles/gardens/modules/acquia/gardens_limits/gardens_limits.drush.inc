<?php

/**
 * @file
 * Drush functionalities related to gardens_limits module.
 */

/**
 * Implements hook_drush_command().
 */
function gardens_limits_drush_command() {
  $items = array();

  $items['gardens-limits-crawler-blocking-disable'] = array(
    'description' => 'Let robots see the site: disables the feature that discourages crawlers from indexing a site.',
    'aliases' => array('glcbd'),
  );
  $items['gardens-limits-crawler-blocking-resume'] = array(
    'description' => 'Hide the site from robots: resumes the disabled feature that discourages crawlers from indexing a site.',
    'aliases' => array('glcbr'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function gardens_limits_drush_help($section) {
  switch ($section) {
    case 'drush:gardens-limits-crawler-blocking-disable':
      return dt('Disables the feature that discourages crawlers from indexing a site.');
    case 'drush:gardens-limits-crawler-blocking-resume':
      return dt('Resumes the disabled feature that discourages crawlers from indexing a site.');
  }
}

/**
 * Disables the feature that discourages crawlers from indexing a site.
 */
function drush_gardens_limits_crawler_blocking_disable() {
  variable_set('gardens_limits_crawler_blocking_killswitch', gardens_limits_crawler_blocking_killswitch());
}

/**
 * Resumes the disabled feature that discourages crawlers from indexing a site.
 */
function drush_gardens_limits_crawler_blocking_resume() {
  variable_del('gardens_limits_crawler_blocking_killswitch');
}
