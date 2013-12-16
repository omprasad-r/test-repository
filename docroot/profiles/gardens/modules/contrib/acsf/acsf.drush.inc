<?php

/**
 * Implements hook_drush_command()
 */
function acsf_drush_command() {
  return array(
    'acsf-install' => array(
      'callback' => 'drush_acsf_install',
      'description' => dt('Installs the required Drupal modules to be compatible with the Acquia Site Factory.'),
      'options' => array(
        'nid' => dt('The node ID from the Site Factory site node.'),
      ),
    ),
    'acsf-registry-rebuild' => array(
      'description' => dt('Rebuilds the ACSF registry.'),
    ),
  );
}

/**
 * Callback: Installs the required ACSF modules.
 */
function drush_acsf_install() {
  $nid = drush_get_option('nid', variable_get('gardens_site_id', NULL));
  acsf_build_registry();

  // Initiate an event to run post-acsf-install tasks.
  $acsf_dir = realpath(dirname(dirname(__FILE__)));
  $type = 'acsf_install';
  $context = array('nid' => $nid);
  $event = AcsfEvent::create($type, $context);
  $event->run();
}

/**
 * Rebuilds the ACSF registry.
 */
function drush_acsf_registry_rebuild() {
  acsf_build_registry();
  drush_print(dt('The ACSF registry was rebuilt.'));
}

