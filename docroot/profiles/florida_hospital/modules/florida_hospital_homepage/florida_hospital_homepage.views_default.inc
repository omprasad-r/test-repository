<?php
/**
 * @file
 * florida_hospital_homepage.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function florida_hospital_homepage_views_default_views() {
  $export = array();

  $view = new view;
  $view->name = 'homepage';
  $view->description = 'Triptych blocks for site homepages';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Homepage';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '3';
  $handler->display->display_options['style_plugin'] = 'grid';
  $handler->display->display_options['style_options']['columns'] = '3';
  $handler->display->display_options['style_options']['fill_single_line'] = 1;
  $handler->display->display_options['row_plugin'] = 'fields';
  $handler->display->display_options['row_options']['hide_empty'] = 0;
  $handler->display->display_options['row_options']['default_field_elements'] = 1;
  /* Field: Content: Teaser Image */
  $handler->display->display_options['fields']['field_title_teaser_image']['id'] = 'field_title_teaser_image';
  $handler->display->display_options['fields']['field_title_teaser_image']['table'] = 'field_data_field_title_teaser_image';
  $handler->display->display_options['fields']['field_title_teaser_image']['field'] = 'field_title_teaser_image';
  $handler->display->display_options['fields']['field_title_teaser_image']['label'] = '';
  $handler->display->display_options['fields']['field_title_teaser_image']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_title_teaser_image']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_title_teaser_image']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_title_teaser_image']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_title_teaser_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_title_teaser_image']['settings'] = array(
    'image_style' => '',
    'image_link' => '',
  );
  $handler->display->display_options['fields']['field_title_teaser_image']['field_api_classes'] = 0;
  /* Field: Content: Teaser More Link Text */
  $handler->display->display_options['fields']['field_more_teaser_text']['id'] = 'field_more_teaser_text';
  $handler->display->display_options['fields']['field_more_teaser_text']['table'] = 'field_data_field_more_teaser_text';
  $handler->display->display_options['fields']['field_more_teaser_text']['field'] = 'field_more_teaser_text';
  $handler->display->display_options['fields']['field_more_teaser_text']['label'] = '';
  $handler->display->display_options['fields']['field_more_teaser_text']['exclude'] = TRUE;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_more_teaser_text']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_more_teaser_text']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_more_teaser_text']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_more_teaser_text']['field_api_classes'] = 0;
  /* Field: Content: Path */
  $handler->display->display_options['fields']['path']['id'] = 'path';
  $handler->display->display_options['fields']['path']['table'] = 'node';
  $handler->display->display_options['fields']['path']['field'] = 'path';
  $handler->display->display_options['fields']['path']['label'] = '';
  $handler->display->display_options['fields']['path']['exclude'] = TRUE;
  $handler->display->display_options['fields']['path']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['path']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['path']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['path']['alter']['external'] = 0;
  $handler->display->display_options['fields']['path']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['path']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['path']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['path']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['path']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['path']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['path']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['path']['alter']['html'] = 0;
  $handler->display->display_options['fields']['path']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['path']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['path']['hide_empty'] = 0;
  $handler->display->display_options['fields']['path']['empty_zero'] = 0;
  $handler->display->display_options['fields']['path']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['path']['absolute'] = 0;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['body']['alter']['text'] = '<div class="teaser-image">[field_title_teaser_image]</div>
<div class="teaser-summary">[body-summary]</div>
<a href="[path]" class="teaser-link button">[field_more_teaser_text]</a>';
  $handler->display->display_options['fields']['body']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['body']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['body']['alter']['external'] = 0;
  $handler->display->display_options['fields']['body']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['body']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['body']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['body']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['body']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['body']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['body']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['body']['alter']['html'] = 0;
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['body']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['body']['hide_empty'] = 0;
  $handler->display->display_options['fields']['body']['empty_zero'] = 0;
  $handler->display->display_options['fields']['body']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['body']['type'] = 'text_summary_or_trimmed';
  $handler->display->display_options['fields']['body']['settings'] = array(
    'trim_length' => '250',
  );
  $handler->display->display_options['fields']['body']['field_api_classes'] = 0;
  /* Sort criterion: Content: Weight (field_weight) */
  $handler->display->display_options['sorts']['field_weight_value']['id'] = 'field_weight_value';
  $handler->display->display_options['sorts']['field_weight_value']['table'] = 'field_data_field_weight';
  $handler->display->display_options['sorts']['field_weight_value']['field'] = 'field_weight_value';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Promoted to front page */
  $handler->display->display_options['filters']['promote']['id'] = 'promote';
  $handler->display->display_options['filters']['promote']['table'] = 'node';
  $handler->display->display_options['filters']['promote']['field'] = 'promote';
  $handler->display->display_options['filters']['promote']['value'] = '1';
  $handler->display->display_options['filters']['promote']['group'] = 1;

  /* Display: Front Page Triptych */
  $handler = $view->new_display('page', 'Front Page Triptych', 'page');
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '3';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['path'] = 'homepage';
  $export['homepage'] = $view;

  return $export;
}
