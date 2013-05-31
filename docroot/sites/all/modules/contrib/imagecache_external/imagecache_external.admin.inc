<?php
/*
 * @file imagecache_external.admin.inc
 * Admin forms for this module
 * @copyright Copyright(c) 2010 Lee Rowlands
 * @license GPL v2 http://www.fsf.org/licensing/licenses/gpl.html
 * @author Lee Rowlands contact at rowlandsgroup dot com
 *
 */
/**
 * Form builder
 */
function imagecache_external_admin_form($form, $form_state) {
  $form = array();
  $form['imagecache_external_option'] = array(
    '#type' => 'radios',
    '#title' => t('Hosts list option'),
    '#description' => t('Choose the method to filter external urls (note admin user is not affected)'),
    '#options' => array(
      'black' => t('Blacklist: Only deny images from the listed hosts.'),
      'white' => t('Whitelist: Deny all images <b>except</b> the listed hosts.'),
    ),
    '#default_value' => variable_get('imagecache_external_option', 'white'),
  );

  $form['imagecache_external_hosts'] = array(
    '#type' => 'textarea',
    '#title' => t('Imagecache External hosts'),
    '#description' => t('Add one host per line. Ex: www.flickr.com'),
    '#default_value' => variable_get('imagecache_external_hosts', ''),
  );
  return system_settings_form($form);
}

/**
 * Form builder
 */
function imagecache_external_flush_form($form, $form_state) {
  return confirm_form($form,
    t('Flush all external images?'),
    'admin/config/media/imagecache_external',
    t('Are you sure, this cannot be undone.'),
    t('Flush'),
    t('Cancel')
  );
}

/**
 * Submit handler
 */
function imagecache_external_flush_form_submit($form, &$form_state) {
  $scheme = file_default_scheme();
  $wrapper= file_stream_wrapper_get_instance_by_scheme($scheme);
  $path   = realpath($wrapper->getDirectoryPath() . '/imagecache');
  if (is_dir($path)) {
    file_unmanaged_delete_recursive($path);
    drupal_set_message(t('Flushed external images'));
    $form_state['redirect'] = 'admin/config/media/imagecache_external';
  }

}
