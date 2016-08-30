<?php

/**
 * Implements hook_form().
 * This form will be displayed in /livefyre-collection-settings
 */

function livefyre_collection_form($form, &$form_state)
{
  $result = db_select('livefyre_collection', 'lc')->fields('lc', array(
    'networkName',
    'networkKey',
    'siteId',
    'siteKey',
    'userId',
    'collectionName'
  ))->OrderBy('nid', 'DESC')->execute()->fetchAssoc();
  
  $networkName    = $result["networkName"];
  $networkKey     = $result["networkKey"];
  $siteId         = $result["siteId"];
  $siteKey        = $result["siteKey"];
  $userId         = $result["userId"];
  $collectionName = $result["collectionName"];
  
  $form['networkName']    = array(
    '#type' => 'textfield',
    '#title' => t('Network Name'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($networkName) ? $networkName : NULL,
    '#required' => TRUE,
    '#description' => t('The name of the Livefyre network (provided by Livefyre).')
  );
  $form['networkKey']     = array(
    '#type' => 'textfield',
    '#title' => t('Network ID'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($networkKey) ? $networkKey : NULL,
    '#required' => TRUE,
    '#description' => t('The secret key for this specific network (provided by Livefyre).')
  );
  $form['siteId']         = array(
    '#type' => 'textfield',
    '#title' => t('Site ID'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($siteId) ? $siteId : NULL,
    '#required' => TRUE,
    '#description' => t('The ID for the site (available from Studio > Settings > Integration Settings > Credentials)(provided by Livefyre).')
  );
  $form['siteKey']        = array(
    '#type' => 'textfield',
    '#title' => t('Site Key'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($siteKey) ? $siteKey : NULL,
    '#required' => TRUE,
    '#description' => t('The secret key for the site (available from Studio > Settings > Integration Settings > Credentials)(provided by Livefyre).')
  );
  $form['userId']         = array(
    '#type' => 'textfield',
    '#title' => t('Livefyre admin user id'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($userId) ? $userId : NULL,
    '#required' => TRUE,
    '#description' => t('The user ID for logging Livefyre (provided by Livefyre).')
  );
  $form['collectionName'] = array(
    '#type' => 'textfield',
    '#title' => t('Artist Name'),
    '#size' => 32,
    '#maxlength' => 255,
    '#default_value' => isset($collectionName) ? $collectionName : NULL,
    '#required' => TRUE,
    '#description' => t('Name of the artist to be used in the livefyre dashboard')
  );
  
  $form['submit'] = array(
    '#value' => 'Submit the Values',
    '#type' => 'submit'
  );
  return $form;
}


/**
 * Implements hook_form_submit().
 */
function livefyre_collection_form_submit($form, &$form_state)
{
  $networkName    = $form_state['values']['networkName'];
  $networkKey     = $form_state['values']['networkKey'];
  $siteId         = $form_state['values']['siteId'];
  $siteKey        = $form_state['values']['siteKey'];
  $userId         = $form_state['values']['userId'];
  $collectionName = $form_state['values']['collectionName'];
  
  $result = db_select('livefyre_collection', 'lc')->fields('lc', array(
    'networkName',
    'networkKey',
    'siteId',
    'siteKey',
    'userId',
    "collectionName"
  ))->OrderBy('nid', 'DESC')->execute()->fetchAssoc();
  
  if (!empty($result["networkName"])) {
    db_update('livefyre_collection')->fields(array(
      'networkName' => $networkName,
      'networkKey' => $networkKey,
      'siteId' => $siteId,
      'siteKey' => $siteKey,
      'userId' => $userId,
      'collectionName' => $collectionName
    ))->execute();
    drupal_set_message(t('Thanks for updating the collection settings'));
  } else {
    db_insert('livefyre_collection')->fields(array(
      'networkName',
      'networkKey',
      'siteId',
      'siteKey',
      'userId',
      'collectionName'
    ))->values(array(
      'networkName' => $networkName,
      'networkKey' => $networkKey,
      'siteId' => $siteId,
      'siteKey' => $siteKey,
      'userId' => $userId,
      'collectionName' => $collectionName
    ))->execute();
    drupal_set_message(t('Thank you for submitting the values for collection settings.'));
  }
}
