<?php

/**
 * Implements hook_views_default_views().
 */
function warner_misc_views_default_views() {
  $views = array();

  $view = new view;
  $view->name = 'user_export';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'users';
  $view->human_name = 'user export';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['access']['type'] = 'role';
  $handler->display->display_options['access']['role'] = array(
    3 => '3',
    4 => '4',
  );
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '5';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['expose']['items_per_page_options_all'] = 0;
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'uid' => 'uid',
    'name' => 'name',
    'status' => 'status',
    'mail' => 'mail',
    'created' => 'created',
    'data' => 'data',
    'field_coppa_lite_dob' => 'field_coppa_lite_dob',
    'language' => 'language',
    'access' => 'access',
    'login' => 'login',
    'pass' => 'pass',
    'picture' => 'picture',
    'rid' => 'rid',
    'signature' => 'signature',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'uid' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'status' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'mail' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'created' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'data' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_coppa_lite_dob' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'language' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'access' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'login' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'pass' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'picture' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'rid' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'signature' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['style_options']['override'] = 1;
  $handler->display->display_options['style_options']['sticky'] = 0;
  $handler->display->display_options['style_options']['empty_table'] = 0;
  /* Header: Global: Text area */
  $handler->display->display_options['header']['area']['id'] = 'area';
  $handler->display->display_options['header']['area']['table'] = 'views';
  $handler->display->display_options['header']['area']['field'] = 'area';
  $handler->display->display_options['header']['area']['empty'] = FALSE;
  $handler->display->display_options['header']['area']['content'] = 'Preview the data to be exported.  Click the "CSV" icon to export.';
  $handler->display->display_options['header']['area']['format'] = 'safe_html';
  $handler->display->display_options['header']['area']['tokenize'] = 0;
  /* Field: User: Uid */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'users';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  $handler->display->display_options['fields']['uid']['label'] = 'uid';
  $handler->display->display_options['fields']['uid']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['external'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['uid']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['uid']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['html'] = 0;
  $handler->display->display_options['fields']['uid']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['uid']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['uid']['hide_empty'] = 0;
  $handler->display->display_options['fields']['uid']['empty_zero'] = 0;
  $handler->display->display_options['fields']['uid']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['uid']['link_to_user'] = 0;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['label'] = 'name';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['name']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['name']['link_to_user'] = 0;
  $handler->display->display_options['fields']['name']['overwrite_anonymous'] = 0;
  $handler->display->display_options['fields']['name']['format_username'] = 1;
  /* Field: User: E-mail */
  $handler->display->display_options['fields']['mail']['id'] = 'mail';
  $handler->display->display_options['fields']['mail']['table'] = 'users';
  $handler->display->display_options['fields']['mail']['field'] = 'mail';
  $handler->display->display_options['fields']['mail']['label'] = 'mail';
  $handler->display->display_options['fields']['mail']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['external'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['mail']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['mail']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['html'] = 0;
  $handler->display->display_options['fields']['mail']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['mail']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['mail']['hide_empty'] = 0;
  $handler->display->display_options['fields']['mail']['empty_zero'] = 0;
  $handler->display->display_options['fields']['mail']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['mail']['link_to_user'] = '0';
  /* Field: User: Created date */
  $handler->display->display_options['fields']['created']['id'] = 'created';
  $handler->display->display_options['fields']['created']['table'] = 'users';
  $handler->display->display_options['fields']['created']['field'] = 'created';
  $handler->display->display_options['fields']['created']['label'] = 'created';
  $handler->display->display_options['fields']['created']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['created']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['created']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['created']['alter']['external'] = 0;
  $handler->display->display_options['fields']['created']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['created']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['created']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['created']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['created']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['created']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['created']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['created']['alter']['html'] = 0;
  $handler->display->display_options['fields']['created']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['created']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['created']['hide_empty'] = 0;
  $handler->display->display_options['fields']['created']['empty_zero'] = 0;
  $handler->display->display_options['fields']['created']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['created']['date_format'] = 'custom';
  $handler->display->display_options['fields']['created']['custom_date_format'] = 'c';
  /* Field: User: Language */
  $handler->display->display_options['fields']['language']['id'] = 'language';
  $handler->display->display_options['fields']['language']['table'] = 'users';
  $handler->display->display_options['fields']['language']['field'] = 'language';
  $handler->display->display_options['fields']['language']['label'] = 'language';
  $handler->display->display_options['fields']['language']['alter']['alter_text'] = 1;
  $handler->display->display_options['fields']['language']['alter']['text'] = '[language]';
  $handler->display->display_options['fields']['language']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['language']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['language']['alter']['external'] = 0;
  $handler->display->display_options['fields']['language']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['language']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['language']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['language']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['language']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['language']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['language']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['language']['alter']['html'] = 0;
  $handler->display->display_options['fields']['language']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['language']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['language']['hide_empty'] = 0;
  $handler->display->display_options['fields']['language']['empty_zero'] = 0;
  $handler->display->display_options['fields']['language']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['language']['link_to_user'] = 0;
  /* Field: User: Last login */
  $handler->display->display_options['fields']['login']['id'] = 'login';
  $handler->display->display_options['fields']['login']['table'] = 'users';
  $handler->display->display_options['fields']['login']['field'] = 'login';
  $handler->display->display_options['fields']['login']['label'] = 'last_login';
  $handler->display->display_options['fields']['login']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['login']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['login']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['login']['alter']['external'] = 0;
  $handler->display->display_options['fields']['login']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['login']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['login']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['login']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['login']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['login']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['login']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['login']['alter']['html'] = 0;
  $handler->display->display_options['fields']['login']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['login']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['login']['hide_empty'] = 0;
  $handler->display->display_options['fields']['login']['empty_zero'] = 0;
  $handler->display->display_options['fields']['login']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['login']['date_format'] = 'custom';
  $handler->display->display_options['fields']['login']['custom_date_format'] = 'c';
  /* Field: User: Password hash */
  $handler->display->display_options['fields']['pass']['id'] = 'pass';
  $handler->display->display_options['fields']['pass']['table'] = 'users';
  $handler->display->display_options['fields']['pass']['field'] = 'pass';
  $handler->display->display_options['fields']['pass']['label'] = 'password';
  $handler->display->display_options['fields']['pass']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['external'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['pass']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['pass']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['pass']['alter']['html'] = 0;
  $handler->display->display_options['fields']['pass']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['pass']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['pass']['hide_empty'] = 0;
  $handler->display->display_options['fields']['pass']['empty_zero'] = 0;
  $handler->display->display_options['fields']['pass']['hide_alter_empty'] = 1;
  /* Field: User: Roles */
  $handler->display->display_options['fields']['rid']['id'] = 'rid';
  $handler->display->display_options['fields']['rid']['table'] = 'users_roles';
  $handler->display->display_options['fields']['rid']['field'] = 'rid';
  $handler->display->display_options['fields']['rid']['label'] = 'roles';
  $handler->display->display_options['fields']['rid']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['external'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['rid']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['rid']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['html'] = 0;
  $handler->display->display_options['fields']['rid']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['rid']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['rid']['hide_empty'] = 0;
  $handler->display->display_options['fields']['rid']['empty_zero'] = 0;
  $handler->display->display_options['fields']['rid']['hide_alter_empty'] = 1;
  /* Field: User: Signature */
  $handler->display->display_options['fields']['signature']['id'] = 'signature';
  $handler->display->display_options['fields']['signature']['table'] = 'users';
  $handler->display->display_options['fields']['signature']['field'] = 'signature';
  $handler->display->display_options['fields']['signature']['label'] = 'signature';
  $handler->display->display_options['fields']['signature']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['external'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['signature']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['signature']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['signature']['alter']['html'] = 0;
  $handler->display->display_options['fields']['signature']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['signature']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['signature']['hide_empty'] = 0;
  $handler->display->display_options['fields']['signature']['empty_zero'] = 0;
  $handler->display->display_options['fields']['signature']['hide_alter_empty'] = 1;
  /* Field: User: Picture URL */
  $handler->display->display_options['fields']['picture_url']['id'] = 'picture_url';
  $handler->display->display_options['fields']['picture_url']['table'] = 'users';
  $handler->display->display_options['fields']['picture_url']['field'] = 'picture_url';
  $handler->display->display_options['fields']['picture_url']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['external'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['picture_url']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['picture_url']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['picture_url']['alter']['html'] = 0;
  $handler->display->display_options['fields']['picture_url']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['picture_url']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['picture_url']['hide_empty'] = 0;
  $handler->display->display_options['fields']['picture_url']['empty_zero'] = 0;
  $handler->display->display_options['fields']['picture_url']['hide_alter_empty'] = 1;
  /* Sort criterion: User: Created date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'users';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: User: Active */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'users';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = '1';

  /* Display: Data export */
  $handler = $view->new_display('views_data_export', 'Data export', 'views_data_export_1');
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1000';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['style_plugin'] = 'views_data_export_csv';
  $handler->display->display_options['style_options']['provide_file'] = 0;
  $handler->display->display_options['style_options']['parent_sort'] = 0;
  $handler->display->display_options['style_options']['quote'] = 1;
  $handler->display->display_options['style_options']['trim'] = 0;
  $handler->display->display_options['style_options']['replace_newlines'] = 0;
  $handler->display->display_options['style_options']['header'] = 1;
  $handler->display->display_options['path'] = 'admin/people/export.csv';
  $handler->display->display_options['displays'] = array(
    'page_1' => 'page_1',
    'default' => 0,
  );
  $handler->display->display_options['use_batch'] = 'batch';
  $handler->display->display_options['segment_size'] = '200';

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->display->display_options['path'] = 'admin/people/export';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Export';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 0;
  $translatables['user_export'] = array(
    t('Master'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('Preview the data to be exported.  Click the "CSV" icon to export.'),
    t('uid'),
    t('name'),
    t('mail'),
    t('created'),
    t('language'),
    t('[language]'),
    t('last_login'),
    t('password'),
    t('roles'),
    t('signature'),
    t('Picture URL'),
    t('Data export'),
    t('Page'),
  );
  // Add view to list of views to provide.
  $views[$view->name] = $view;

  $view = new view;
  $view->name = 'members';
  $view->description = 'Displays filterable information about the members of the site.';
  $view->tag = 'default';
  $view->base_table = 'users';
  $view->human_name = 'Members';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  
  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'People';
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'administer users';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Filter';
  $handler->display->display_options['exposed_form']['options']['reset_button'] = TRUE;
  $handler->display->display_options['exposed_form']['options']['autosubmit'] = 0;
  $handler->display->display_options['exposed_form']['options']['autosubmit_hide'] = 1;
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '25';
  $handler->display->display_options['style_plugin'] = 'table';
  /* Field: User: Uid */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'users';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  $handler->display->display_options['fields']['uid']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['external'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['uid']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['uid']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['uid']['alter']['html'] = 0;
  $handler->display->display_options['fields']['uid']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['uid']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['uid']['hide_empty'] = 0;
  $handler->display->display_options['fields']['uid']['empty_zero'] = 0;
  $handler->display->display_options['fields']['uid']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['uid']['link_to_user'] = 1;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['label'] = 'Username';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 0;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 0;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['name']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  $handler->display->display_options['fields']['name']['overwrite_anonymous'] = 0;
  $handler->display->display_options['fields']['name']['format_username'] = 1;
  /* Field: User: E-mail */
  $handler->display->display_options['fields']['mail']['id'] = 'mail';
  $handler->display->display_options['fields']['mail']['table'] = 'users';
  $handler->display->display_options['fields']['mail']['field'] = 'mail';
  $handler->display->display_options['fields']['mail']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['external'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['mail']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['mail']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['mail']['alter']['html'] = 0;
  $handler->display->display_options['fields']['mail']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['mail']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['mail']['hide_empty'] = 0;
  $handler->display->display_options['fields']['mail']['empty_zero'] = 0;
  $handler->display->display_options['fields']['mail']['hide_alter_empty'] = 1;
  /* Field: User: Country */
  $handler->display->display_options['fields']['field_country']['id'] = 'field_country';
  $handler->display->display_options['fields']['field_country']['table'] = 'field_data_field_country';
  $handler->display->display_options['fields']['field_country']['field'] = 'field_country';
  $handler->display->display_options['fields']['field_country']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_country']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_country']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_country']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_country']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['field_country']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_country']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_country']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_country']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_country']['field_api_classes'] = 0;
  /* Field: User: Gender */
  $handler->display->display_options['fields']['field_gender']['id'] = 'field_gender';
  $handler->display->display_options['fields']['field_gender']['table'] = 'field_data_field_gender';
  $handler->display->display_options['fields']['field_gender']['field'] = 'field_gender';
  $handler->display->display_options['fields']['field_gender']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_gender']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_gender']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_gender']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_gender']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['field_gender']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_gender']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_gender']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_gender']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_gender']['field_api_classes'] = 0;
  /* Field: User: Picture */
  $handler->display->display_options['fields']['picture']['id'] = 'picture';
  $handler->display->display_options['fields']['picture']['table'] = 'users';
  $handler->display->display_options['fields']['picture']['field'] = 'picture';
  $handler->display->display_options['fields']['picture']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['external'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['picture']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['picture']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['picture']['alter']['html'] = 0;
  $handler->display->display_options['fields']['picture']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['picture']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['picture']['hide_empty'] = 0;
  $handler->display->display_options['fields']['picture']['empty_zero'] = 0;
  $handler->display->display_options['fields']['picture']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['picture']['link_photo_to_profile'] = 1;
  /* Field: User: Created date */
  $handler->display->display_options['fields']['created']['id'] = 'created';
  $handler->display->display_options['fields']['created']['table'] = 'users';
  $handler->display->display_options['fields']['created']['field'] = 'created';
  $handler->display->display_options['fields']['created']['label'] = 'Member since';
  $handler->display->display_options['fields']['created']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['created']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['created']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['created']['alter']['external'] = 0;
  $handler->display->display_options['fields']['created']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['created']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['created']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['created']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['created']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['created']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['created']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['created']['alter']['html'] = 0;
  $handler->display->display_options['fields']['created']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['created']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['created']['hide_empty'] = 0;
  $handler->display->display_options['fields']['created']['empty_zero'] = 0;
  $handler->display->display_options['fields']['created']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['created']['date_format'] = 'long';
  /* Field: User: Roles */
  $handler->display->display_options['fields']['rid']['id'] = 'rid';
  $handler->display->display_options['fields']['rid']['table'] = 'users_roles';
  $handler->display->display_options['fields']['rid']['field'] = 'rid';
  $handler->display->display_options['fields']['rid']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['external'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['rid']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['rid']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['rid']['alter']['html'] = 0;
  $handler->display->display_options['fields']['rid']['element_label_colon'] = 0;
  $handler->display->display_options['fields']['rid']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['rid']['hide_empty'] = 0;
  $handler->display->display_options['fields']['rid']['empty_zero'] = 0;
  $handler->display->display_options['fields']['rid']['hide_alter_empty'] = 1;
  /* Sort criterion: User: Created date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'users';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: User: E-mail */
  $handler->display->display_options['filters']['mail']['id'] = 'mail';
  $handler->display->display_options['filters']['mail']['table'] = 'users';
  $handler->display->display_options['filters']['mail']['field'] = 'mail';
  $handler->display->display_options['filters']['mail']['operator'] = 'contains';
  $handler->display->display_options['filters']['mail']['group'] = 1;
  $handler->display->display_options['filters']['mail']['exposed'] = TRUE;
  $handler->display->display_options['filters']['mail']['expose']['operator_id'] = 'mail_op';
  $handler->display->display_options['filters']['mail']['expose']['label'] = 'E-mail';
  $handler->display->display_options['filters']['mail']['expose']['operator'] = 'mail_op';
  $handler->display->display_options['filters']['mail']['expose']['identifier'] = 'mail';
  $handler->display->display_options['filters']['mail']['expose']['required'] = 0;
  $handler->display->display_options['filters']['mail']['expose']['multiple'] = FALSE;
  /* Filter criterion: User: Name (raw) */
  $handler->display->display_options['filters']['name']['id'] = 'name';
  $handler->display->display_options['filters']['name']['table'] = 'users';
  $handler->display->display_options['filters']['name']['field'] = 'name';
  $handler->display->display_options['filters']['name']['operator'] = 'contains';
  $handler->display->display_options['filters']['name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['name']['expose']['operator_id'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['label'] = 'Name';
  $handler->display->display_options['filters']['name']['expose']['operator'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['identifier'] = 'name';
  $handler->display->display_options['filters']['name']['expose']['required'] = 0;
  $handler->display->display_options['filters']['name']['expose']['multiple'] = FALSE;
  /* Filter criterion: User: Name (raw) */
  $handler->display->display_options['filters']['name_1']['id'] = 'name_1';
  $handler->display->display_options['filters']['name_1']['table'] = 'users';
  $handler->display->display_options['filters']['name_1']['field'] = 'name';
  $handler->display->display_options['filters']['name_1']['operator'] = 'longerthan';
  $handler->display->display_options['filters']['name_1']['value'] = '0';
  
  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'admin/people/stats';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Statistics';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 0;
  $handler->display->display_options['tab_options']['weight'] = '0';
  $translatables['members'] = array(
    t('Master'),
    t('People'),
    t('more'),
    t('Filter'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('Uid'),
    t('Username'),
    t('E-mail'),
    t('Country'),
    t('Gender'),
    t('Picture'),
    t('Member since'),
    t('Roles'),
    t('Name'),
    t('Page'),
  );

  // Add view to list of views to provide.
  $views[$view->name] = $view;

  // Audio Example Feed view.
  $view = new view;
  $view->name = 'itunes_example_feed';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'iTunes Audio Example Feed';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = TRUE; /* Edit this to true to make a default view disabled initially */
  
  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'iTunes Audio Example Feed';
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Relationship: Content: Author */
  $handler->display->display_options['relationships']['uid']['id'] = 'uid';
  $handler->display->display_options['relationships']['uid']['table'] = 'node';
  $handler->display->display_options['relationships']['uid']['field'] = 'uid';
  $handler->display->display_options['relationships']['uid']['required'] = 0;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['title']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['title']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['title']['alter']['external'] = 0;
  $handler->display->display_options['fields']['title']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['title']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['title']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = 0;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = 0;
  $handler->display->display_options['fields']['title']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['title']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['title']['alter']['html'] = 0;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['title']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['title']['hide_empty'] = 0;
  $handler->display->display_options['fields']['title']['empty_zero'] = 0;
  $handler->display->display_options['fields']['title']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['title']['link_to_node'] = 0;
  /* Field: Content: Audio asset */
  $handler->display->display_options['fields']['field_audio_asset']['id'] = 'field_audio_asset';
  $handler->display->display_options['fields']['field_audio_asset']['table'] = 'field_data_field_audio_asset';
  $handler->display->display_options['fields']['field_audio_asset']['field'] = 'field_audio_asset';
  $handler->display->display_options['fields']['field_audio_asset']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['field_audio_asset']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_audio_asset']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_audio_asset']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_audio_asset']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_audio_asset']['type'] = 'enclosure';
  $handler->display->display_options['fields']['field_audio_asset']['settings'] = array(
    'file_view_mode' => 'default',
  );
  $handler->display->display_options['fields']['field_audio_asset']['field_api_classes'] = 0;
  /* Field: Content: Body */
  $handler->display->display_options['fields']['body']['id'] = 'body';
  $handler->display->display_options['fields']['body']['table'] = 'field_data_body';
  $handler->display->display_options['fields']['body']['field'] = 'body';
  $handler->display->display_options['fields']['body']['label'] = '';
  $handler->display->display_options['fields']['body']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['body']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['body']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['body']['alter']['external'] = 0;
  $handler->display->display_options['fields']['body']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['body']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['body']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['body']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['body']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['body']['alter']['strip_tags'] = 1;
  $handler->display->display_options['fields']['body']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['body']['alter']['html'] = 0;
  $handler->display->display_options['fields']['body']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['body']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['body']['hide_empty'] = 0;
  $handler->display->display_options['fields']['body']['empty_zero'] = 0;
  $handler->display->display_options['fields']['body']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['body']['type'] = 'text_trimmed';
  $handler->display->display_options['fields']['body']['settings'] = array(
    'trim_length' => '600',
  );
  $handler->display->display_options['fields']['body']['field_api_classes'] = 0;
  /* Field: Content: Duration */
  $handler->display->display_options['fields']['field_audio_duration']['id'] = 'field_audio_duration';
  $handler->display->display_options['fields']['field_audio_duration']['table'] = 'field_data_field_audio_duration';
  $handler->display->display_options['fields']['field_audio_duration']['field'] = 'field_audio_duration';
  $handler->display->display_options['fields']['field_audio_duration']['label'] = '';
  $handler->display->display_options['fields']['field_audio_duration']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_audio_duration']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_audio_duration']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_audio_duration']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_audio_duration']['field_api_classes'] = 0;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['relationship'] = 'uid';
  $handler->display->display_options['fields']['name']['label'] = '';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['name']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['name']['link_to_user'] = 0;
  $handler->display->display_options['fields']['name']['overwrite_anonymous'] = 0;
  $handler->display->display_options['fields']['name']['format_username'] = 1;
  /* Field: Content: Audio image */
  $handler->display->display_options['fields']['field_audio_image']['id'] = 'field_audio_image';
  $handler->display->display_options['fields']['field_audio_image']['table'] = 'field_data_field_audio_image';
  $handler->display->display_options['fields']['field_audio_image']['field'] = 'field_audio_image';
  $handler->display->display_options['fields']['field_audio_image']['label'] = '';
  $handler->display->display_options['fields']['field_audio_image']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['external'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['field_audio_image']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['field_audio_image']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['alter']['html'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_audio_image']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['field_audio_image']['hide_empty'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['empty_zero'] = 0;
  $handler->display->display_options['fields']['field_audio_image']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['field_audio_image']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_audio_image']['type'] = 'itunes:image';
  $handler->display->display_options['fields']['field_audio_image']['settings'] = array(
    'image_style' => '',
  );
  $handler->display->display_options['fields']['field_audio_image']['field_api_classes'] = 0;
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
    'audio' => 'audio',
  );
  
  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->display->display_options['path'] = 'itunes-example-rss';
  
  /* Display: Feed */
  $handler = $view->new_display('feed', 'Feed', 'feed_1');
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['style_plugin'] = 'rss_fields';
  $handler->display->display_options['style_options']['channel'] = array(
    'core' => array(
      'views_rss_core' => array(
        'description' => 'All About Everything is a show about everything. Each week we dive into any subject known to man and talk about it as much as we can. Look for our Podcast in the iTunes Store',
        'language' => 'en-us',
        'category' => '',
        'image' => '',
        'copyright' => '',
        'managingEditor' => '',
        'webMaster' => '',
        'generator' => '',
        'docs' => '',
        'cloud' => '',
        'ttl' => '',
        'skipHours' => '',
        'skipDays' => '',
      ),
    ),
    'itunes' => array(
      'views_rss_itunes' => array(
        'subtitle' => 'A show about everything',
        'summary' => 'All About Everything is a show about everything. Each week we dive into any subject known to man and talk about it as much as we can. Look for our Podcast in the iTunes Store',
        'category' => array(
          306 => '306',
          257 => '257',
        ),
        'keywords' => '',
        'image' => 'sites/default/files/AllAboutEverything.jpg',
        'author' => 'John Doe',
        'explicit' => '',
        'block' => '',
        'complete' => 'no',
        'new-feed-url' => '',
        'owner' => 'john.doe@example.com, John Doe',
      ),
    ),
  );
  $handler->display->display_options['style_options']['item'] = array(
    'core' => array(
      'views_rss_core' => array(
        'title' => 'title',
        'link' => '',
        'description' => '',
        'author' => '',
        'category' => '',
        'comments' => '',
        'enclosure' => 'field_audio_asset',
        'guid' => '',
        'pubDate' => '',
      ),
    ),
    'itunes' => array(
      'views_rss_itunes' => array(
        'subtitle' => 'title',
        'summary' => 'body',
        'duration' => 'field_audio_duration',
        'keywords' => 'field_audio_category',
        'image' => 'field_audio_image',
        'author' => 'name',
        'isClosedCaptioned' => '',
        'explicit' => '',
        'block' => '',
        'order' => '',
      ),
    ),
  );
  $handler->display->display_options['style_options']['feed_settings'] = array(
    'absolute_paths' => 1,
    'feed_in_links' => 0,
  );
  $handler->display->display_options['path'] = 'itunes-example-rss/feed';
  $handler->display->display_options['displays'] = array(
    'page_1' => 'page_1',
    'default' => 0,
  );
  $handler->display->display_options['sitename_title'] = 0;
  $translatables['itunes_example_feed'] = array(
    t('Master'),
    t('iTunes Audio Example Feed'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('author'),
    t('Audio asset'),
    t('Page'),
    t('Feed'),
  );

    // Add view to list of views to provide.
  $views[$view->name] = $view;

  return $views;
}
