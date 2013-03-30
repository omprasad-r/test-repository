<?php

/**
 * @file
 *   Default rules configurations for the antivirus module.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function antivirus_default_rules_configuration() {
  $items = array();

  $items['rules_virus_detected'] = entity_import('rules_config', '{ "rules_virus_detected" : {
      "LABEL" : "Virus detected",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "antivirus" ],
      "REQUIRES" : [ "antivirus" ],
      "ON" : [ "virus_detected" ],
      "DO" : []
    }
  }');

  return $items;
}
