<?php

/**
 * @file
 * Includes any rules integration provided by the Antivirus module.
 */

/**
 * Implementats hook_rules_event_info().
 */
function antivirus_rules_event_info() {
  return array(
    'virus_detected' => array(
      'group' => t('antivirus'),
      'module' => 'antivirus',
      'access callback' => 'rules_comment_integration_access',
      'label' => t('After detecting a virus'),
      'variables' => array(
        'file' => array(
          'type' => 'file',
          'label' => t('infected file'),
        ),
      ),
    ),
  );
}

