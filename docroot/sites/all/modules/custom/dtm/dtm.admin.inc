<?php

/**
 * @file
 * This file contains all the admin-related callbacks
 */


/**
 * Overview page - provides a list of existing add to head profiles
 */
function dtm_overview() {
  // Fetch the list of profiles saved to the DB.
  $settings = variable_get('dtm_profiles', array());

  // Allow other modules to alter them.
  drupal_alter(basename(__FILE__, '.admin.inc') . '_profiles', $settings);

  // Set the titles in the header.
  $header = array(t('Title'), t('Scope'), t('Ops'));

  // List each profile on the overview page.
  $rows = array();
  foreach ($settings as $delta => $settings) {

    // Show all possible operations on the profile.
    if (in_array($settings, variable_get('dtm_profiles', array()))) {
      // This profile is in the DB. It can be modified through the Web UI.
      $ops = implode(' | ', array(
          l(t('Edit'), 'admin/config/development/dtm/' . $delta),
          l(t('Delete'), 'admin/config/development/dtm/' . $delta . '/delete'),
      ));
    }
    else {
      // The profile is in code only. It cannot be edited from here so show a message.
      $ops = t('None (in code)');
    }

    // Fill the current row with profile data.
    $rows[] = array(
      check_plain($settings['name']),
      drupal_ucfirst($settings['scope']),
      $ops,
    );
  }

  if (!empty($rows)) {
    $rows[] = array(array(
      'colspan' => count($header),
      'data' => l(t('Add another profile'), 'admin/config/development/dtm/add'),
    ));
  }

  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No profiles configured yet. !link', array('!link' => l(t('Add one now'), 'admin/config/development/dtm/add')))
  );
}


/**
 * This function provides the edit form.
 * The Add Profile form also uses this.
 * @see dtm_forms()
 */
function dtm_edit_profile($form, &$form_state, $profile = array()) {
  // Fill in profile defaults to ensure all keys exist.
  $profile += array(
    'name' => '',
    'code' => '',
    'path_visibility' => '',
    'paths' => '',
    'scope' => '',
    'scope' => '',
    'dtmcodeheader' => '',
  );
  $form['name_orig'] = array(
    '#type' => 'value',
    '#value' => $profile['name'],
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('This is the unique name for this profile'),
    '#required' => TRUE,
    '#default_value' => $profile['name'],
  );
  $form['dtmcodeheader'] = array(
    '#type' => 'textarea',
    '#title' => t('DTM Script Code'),
    '#description' => t('Enter the code you would like to insert into the header of the page. <br> Example: http://assets.adobedtm.com/31543e7db99435a92d6f4a2cf97c808672ed7dd0/satelliteLib-b6e3529c31d92a9252c30c1714b6b9342304145b.js'),
    '#required' => TRUE,
    '#default_value' => variable_get(t('http://assets.adobedtm.com/31543e7db99435a92d6f4a2cf97c808672ed7dd0/satelliteLib-b6e3529c31d92a9252c30c1714b6b9342304145b.js'), $profile['dtmcodeheader']),
  );
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => t('Extend Code'),
    '#description' => t('Enter the code you would like to insert into the footer of the code.<br> Example:digitalData.page.pageInfo.pageName:"[node:title]"'),
  '#required' => TRUE,
  '#default_value' => $profile['code'], 
  );
   $form['token_help'] = array(
        '#title' => t('Replacement patterns'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
     );

   $form['token_help']['help'] = array(
        '#markup' => theme('token_tree', array('token_types' => array('node'))),
      );
   $form['scope'] = array(
    '#type' => 'hidden',
    '#title' => t('Scope of addition'),
    '#description' => t('Which section of the head would you like this snippet appended to?'),
    '#options' => array(
      'head' => t('Head - Enble this to include the js in the header'),
      'page_bottom' => t('Footer - Enble this to include the js in the footer'),
    ),
    '#default_value' => "head",
    '#required' => TRUE,
  );


  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


/**
 * Validate handler for the add/edit form
 */
function dtm_edit_profile_validate($form, &$form_state) {
  $settings = variable_get('dtm_profiles', array());

  if (preg_match('/[^a-z0-9\-]/', $form_state['values']['name'])) {
    form_set_error('name', t('The name should only contain lower case letters, numbers and hyphens.'));
  }
  elseif ( ($form_state['values']['name'] != $form_state['values']['name_orig']) && (isset($settings[$form_state['values']['name']])) ) {
    form_set_error('name', t('This name has already been used. Please try another.'));
  }
}


/**
 * Submit handler for the add/edit form
 */
function dtm_edit_profile_submit($form, &$form_state) {
  $settings = variable_get('dtm_profiles', array());

  if ($form_state['values']['name'] != $form_state['values']['name_orig']) {
    unset($settings[$form_state['values']['name_orig']]);
  }


  $settings[$form_state['values']['name']] = array(
    'name'  => $form_state['values']['name'],
    'code'  => trim($form_state['values']['code']),
    'scope' => $form_state['values']['scope'],
    'dtmcodeheader' => $form_state['values']['dtmcodeheader'],
  );

  variable_set('dtm_profiles', $settings);

  $form_state['redirect'] = 'admin/config/development/dtm';
}


/**
 * Delete confirm form for removing a profile
 */
function dtm_delete_profile_confirm($form, &$form_state, $profile) {
  $form['#profile'] = $profile;

  return confirm_form($form,
    t('Are you sure you want to delete the profile %name', array('%name' => $profile['name'])),
    'admin/config/development/dtm',
    t('This cannot be undone'),
    t('Delete'), t('Cancel')
  );
}


/**
 * Delete confirm form submit handler for removing a profile.
 */
function dtm_delete_profile_confirm_submit($form, &$form_state) {
  $settings = variable_get('dtm_profiles', array());

  unset($settings[$form['#profile']['name']]);

  variable_set('dtm_profiles', $settings);

  $form_state['redirect'] = 'admin/config/development/dtm';
}


