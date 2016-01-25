<?php
/**
 * @file
 * turnkey_toaster.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function turnkey_toaster_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'turnkey_lightbox';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Turnkey Lightbox';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Turnkey Lightbox';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['style_plugin'] = 'views_json';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: Label List ID */
  $handler->display->display_options['fields']['field_label_list_id']['id'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['table'] = 'field_data_field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['field'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['label'] = '';
  $handler->display->display_options['fields']['field_label_list_id']['element_label_colon'] = FALSE;
  /* Field: Content: Preview Mode */
  $handler->display->display_options['fields']['field_preview_mode']['id'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['table'] = 'field_data_field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['field'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['label'] = '';
  $handler->display->display_options['fields']['field_preview_mode']['element_label_colon'] = FALSE;
  /* Field: Content: Newsletter ID */
  $handler->display->display_options['fields']['field_newsletter_id']['id'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['table'] = 'field_data_field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['field'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['label'] = '';
  $handler->display->display_options['fields']['field_newsletter_id']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rates']['id'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['table'] = 'field_data_field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['field'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['label'] = '';
  $handler->display->display_options['fields']['field_display_rates']['element_label_colon'] = FALSE;
  /* Field: Content: Timeout */
  $handler->display->display_options['fields']['field_toaster_timeout']['id'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['table'] = 'field_data_field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['field'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_timeout']['element_label_colon'] = FALSE;
  /* Field: Content: Toaster Position */
  $handler->display->display_options['fields']['field_toaster_position']['id'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['table'] = 'field_data_field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['field'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_position']['element_label_colon'] = FALSE;
  /* Field: Content: Distance from Position */
  $handler->display->display_options['fields']['field_turnkey_height']['id'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['table'] = 'field_data_field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['field'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['label'] = '';
  $handler->display->display_options['fields']['field_turnkey_height']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_turnkey_height']['settings'] = array(
    'thousand_separator' => ' ',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Lightbox Width */
  $handler->display->display_options['fields']['field_lbox_width']['id'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['table'] = 'field_data_field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['field'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_width']['element_label_colon'] = FALSE;
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'turnkey_toaster' => 'turnkey_toaster',
  );

  /* Display: Turnkey Toaster JSON */
  $handler = $view->new_display('block', 'Turnkey Toaster JSON', 'block_1');
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'views_json';
  $handler->display->display_options['style_options']['root_object'] = '';
  $handler->display->display_options['style_options']['top_child_object'] = 'parent';
  $handler->display->display_options['style_options']['plaintext_output'] = 1;
  $handler->display->display_options['style_options']['remove_newlines'] = 1;
  $handler->display->display_options['style_options']['jsonp_prefix'] = '';
  $handler->display->display_options['style_options']['content_type'] = 'text/json';
  $handler->display->display_options['style_options']['using_views_api_mode'] = 1;
  $handler->display->display_options['style_options']['object_arrays'] = 0;
  $handler->display->display_options['style_options']['numeric_strings'] = 0;
  $handler->display->display_options['style_options']['bigint_string'] = 0;
  $handler->display->display_options['style_options']['pretty_print'] = 0;
  $handler->display->display_options['style_options']['unescaped_slashes'] = 0;
  $handler->display->display_options['style_options']['unescaped_unicode'] = 0;
  $handler->display->display_options['style_options']['char_encoding'] = array();
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: Newsletter ID */
  $handler->display->display_options['fields']['field_newsletter_id']['id'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['table'] = 'field_data_field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['field'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['label'] = '';
  $handler->display->display_options['fields']['field_newsletter_id']['element_label_colon'] = FALSE;
  /* Field: Content: Label List ID */
  $handler->display->display_options['fields']['field_label_list_id']['id'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['table'] = 'field_data_field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['field'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['label'] = '';
  $handler->display->display_options['fields']['field_label_list_id']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rates']['id'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['table'] = 'field_data_field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['field'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['label'] = '';
  $handler->display->display_options['fields']['field_display_rates']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_display_rates']['type'] = 'list_key';
  /* Field: Content: Preview Mode */
  $handler->display->display_options['fields']['field_preview_mode']['id'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['table'] = 'field_data_field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['field'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['label'] = '';
  $handler->display->display_options['fields']['field_preview_mode']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox Height */
  $handler->display->display_options['fields']['field_lbox_height']['id'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['table'] = 'field_data_field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['field'] = 'field_lbox_height';
  $handler->display->display_options['fields']['field_lbox_height']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_height']['element_label_colon'] = FALSE;
  /* Field: Content: Lightbox Width */
  $handler->display->display_options['fields']['field_lbox_width']['id'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['table'] = 'field_data_field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['field'] = 'field_lbox_width';
  $handler->display->display_options['fields']['field_lbox_width']['label'] = '';
  $handler->display->display_options['fields']['field_lbox_width']['element_label_colon'] = FALSE;
  /* Field: Content: Timeout */
  $handler->display->display_options['fields']['field_toaster_timeout']['id'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['table'] = 'field_data_field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['field'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_timeout']['element_label_colon'] = FALSE;
  /* Field: Content: Toaster Position */
  $handler->display->display_options['fields']['field_toaster_position']['id'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['table'] = 'field_data_field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['field'] = 'field_toaster_position';
  $handler->display->display_options['fields']['field_toaster_position']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_position']['element_label_colon'] = FALSE;
  /* Field: Content: Distance from Position */
  $handler->display->display_options['fields']['field_turnkey_height']['id'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['table'] = 'field_data_field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['field'] = 'field_turnkey_height';
  $handler->display->display_options['fields']['field_turnkey_height']['label'] = '';
  $handler->display->display_options['fields']['field_turnkey_height']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_turnkey_height']['settings'] = array(
    'thousand_separator' => '',
    'prefix_suffix' => 1,
  );
  $handler->display->display_options['block_description'] = 'Turnkey Toaster JSON';

  /* Display: Turnkey Toaster */
  $handler = $view->new_display('block', 'Turnkey Toaster', 'block_2');
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'list';
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['row_plugin'] = 'fields';
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  /* Field: Content: Label List ID */
  $handler->display->display_options['fields']['field_label_list_id']['id'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['table'] = 'field_data_field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['field'] = 'field_label_list_id';
  $handler->display->display_options['fields']['field_label_list_id']['label'] = '';
  $handler->display->display_options['fields']['field_label_list_id']['element_label_colon'] = FALSE;
  /* Field: Content: Preview Mode */
  $handler->display->display_options['fields']['field_preview_mode']['id'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['table'] = 'field_data_field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['field'] = 'field_preview_mode';
  $handler->display->display_options['fields']['field_preview_mode']['label'] = '';
  $handler->display->display_options['fields']['field_preview_mode']['element_label_colon'] = FALSE;
  /* Field: Content: Newsletter ID */
  $handler->display->display_options['fields']['field_newsletter_id']['id'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['table'] = 'field_data_field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['field'] = 'field_newsletter_id';
  $handler->display->display_options['fields']['field_newsletter_id']['label'] = '';
  $handler->display->display_options['fields']['field_newsletter_id']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rates']['id'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['table'] = 'field_data_field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['field'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates']['label'] = '';
  $handler->display->display_options['fields']['field_display_rates']['element_label_colon'] = FALSE;
  /* Field: Content: Timeout */
  $handler->display->display_options['fields']['field_toaster_timeout']['id'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['table'] = 'field_data_field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['field'] = 'field_toaster_timeout';
  $handler->display->display_options['fields']['field_toaster_timeout']['label'] = '';
  $handler->display->display_options['fields']['field_toaster_timeout']['element_label_colon'] = FALSE;
  /* Field: Content: Display rate */
  $handler->display->display_options['fields']['field_display_rates_1']['id'] = 'field_display_rates_1';
  $handler->display->display_options['fields']['field_display_rates_1']['table'] = 'field_data_field_display_rates';
  $handler->display->display_options['fields']['field_display_rates_1']['field'] = 'field_display_rates';
  $handler->display->display_options['fields']['field_display_rates_1']['label'] = '';
  $handler->display->display_options['fields']['field_display_rates_1']['element_label_colon'] = FALSE;
  $handler->display->display_options['block_description'] = 'Turnkey Toaster';
  $translatables['turnkey_lightbox'] = array(
    t('Master'),
    t('Turnkey Lightbox'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Turnkey Toaster JSON'),
    t('Turnkey Toaster'),
  );
  $export['turnkey_lightbox'] = $view;

  return $export;
}
