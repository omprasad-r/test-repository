<?php

/**
 * @file flag_friend.views_default.inc
 *
 * Default view for flag friend which provides a friend listing, a pending
 * friends listing, and a listing of people you've requested as friends.
 */

/**
 * Implements hook_views_default_views().
 */
function flag_friend_views_default_views() {
  $view = new view;
  $view->name = 'friends';
  $view->description = 'Various page displays for flag_friend.';
  $view->tag = 'flag.friend';
  $view->base_table = 'users';
  $view->human_name = '';
  $view->api_version = '3.0-alpha1';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Defaults */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['distinct'] = TRUE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'picture' => 'picture',
    'name' => 'picture',
    'message' => 'message',
    'ops' => 'ops',
    'uid' => 'ops',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'picture' => array(
      'sortable' => 0,
      'separator' => '',
    ),
    'name' => array(
      'sortable' => 0,
      'separator' => '',
    ),
    'message' => array(
      'separator' => '',
    ),
    'ops' => array(
      'separator' => '',
    ),
    'uid' => array(
      'separator' => '',
    ),
  );
  $handler->display->display_options['style_options']['override'] = 1;
  $handler->display->display_options['style_options']['sticky'] = 0;
  /* Empty text: Global: Text area */
  $handler->display->display_options['empty']['text']['id'] = 'area';
  $handler->display->display_options['empty']['text']['table'] = 'views';
  $handler->display->display_options['empty']['text']['field'] = 'area';
  $handler->display->display_options['empty']['text']['empty'] = FALSE;
  $handler->display->display_options['empty']['text']['content'] = 'No Friends have been added.';
  $handler->display->display_options['empty']['text']['format'] = '1';
  /* Relationship: Flag Friend: friend */
  $handler->display->display_options['relationships']['flag_friend_content_rel']['id'] = 'flag_friend_content_rel';
  $handler->display->display_options['relationships']['flag_friend_content_rel']['table'] = 'users';
  $handler->display->display_options['relationships']['flag_friend_content_rel']['field'] = 'flag_friend_content_rel';
  $handler->display->display_options['relationships']['flag_friend_content_rel']['flag'] = 'friend';
  $handler->display->display_options['relationships']['flag_friend_content_rel']['user_scope'] = 'any';
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  /* Field: Flags: Flag link */
  $handler->display->display_options['fields']['ops']['id'] = 'ops';
  $handler->display->display_options['fields']['ops']['table'] = 'flag_content';
  $handler->display->display_options['fields']['ops']['field'] = 'ops';
  $handler->display->display_options['fields']['ops']['relationship'] = 'flag_friend_content_rel';
  /* Field: Flag friend: Flag friend links */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'flag_friend';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  /* Argument: Flag friend: Friends of */
  $handler->display->display_options['arguments']['friend_uid']['id'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['table'] = 'flag_friend';
  $handler->display->display_options['arguments']['friend_uid']['field'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['friend_uid']['default_argument_type'] = 'fixed';
  /* Filter: User: Active */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'users';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = '1';
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;

  /* Display: Page (friends) */
  $handler = $view->new_display('page', 'Page (friends)', 'page_1');
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'Friends';
  $handler->display->display_options['defaults']['relationships'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: User: Picture */
  $handler->display->display_options['fields']['picture']['id'] = 'picture';
  $handler->display->display_options['fields']['picture']['table'] = 'users';
  $handler->display->display_options['fields']['picture']['field'] = 'picture';
  $handler->display->display_options['fields']['picture']['label'] = 'Friend';
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  /* Field: Flag friend: Flag friend links */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'flag_friend';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  $handler->display->display_options['fields']['uid']['label'] = 'Actions';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: Flag friend: Friends of */
  $handler->display->display_options['arguments']['friend_uid']['id'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['table'] = 'flag_friend';
  $handler->display->display_options['arguments']['friend_uid']['field'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['friend_uid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['friend_uid']['default_argument_type'] = 'user';
  $handler->display->display_options['arguments']['friend_uid']['default_argument_options']['user'] = FALSE;
  $handler->display->display_options['arguments']['friend_uid']['break_phrase'] = 0;
  $handler->display->display_options['arguments']['friend_uid']['not'] = 0;
  $handler->display->display_options['path'] = 'user/%/friends/all';
  $handler->display->display_options['menu']['type'] = 'default tab';
  $handler->display->display_options['menu']['title'] = 'View All Friends';
  $handler->display->display_options['menu']['weight'] = '-10';
  $handler->display->display_options['tab_options']['type'] = 'tab';
  $handler->display->display_options['tab_options']['title'] = 'Friends';
  $handler->display->display_options['tab_options']['weight'] = '0';

  /* Display: Page (pending) */
  $handler = $view->new_display('page', 'Page (pending)', 'page_2');
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'Friend Requests';
  $handler->display->display_options['defaults']['header'] = FALSE;
  /* Header: Global: Text area */
  $handler->display->display_options['header']['text']['id'] = 'area';
  $handler->display->display_options['header']['text']['table'] = 'views';
  $handler->display->display_options['header']['text']['field'] = 'area';
  $handler->display->display_options['header']['text']['empty'] = FALSE;
  $handler->display->display_options['header']['text']['content'] = 'These are users who would like to be your friend.';
  $handler->display->display_options['header']['text']['format'] = '1';
  $handler->display->display_options['defaults']['empty'] = FALSE;
  /* Empty text: Global: Text area */
  $handler->display->display_options['empty']['text']['id'] = 'area';
  $handler->display->display_options['empty']['text']['table'] = 'views';
  $handler->display->display_options['empty']['text']['field'] = 'area';
  $handler->display->display_options['empty']['text']['empty'] = FALSE;
  $handler->display->display_options['empty']['text']['content'] = 'No Friend Requests.';
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: User: Picture */
  $handler->display->display_options['fields']['picture']['id'] = 'picture';
  $handler->display->display_options['fields']['picture']['table'] = 'users';
  $handler->display->display_options['fields']['picture']['field'] = 'picture';
  $handler->display->display_options['fields']['picture']['label'] = 'User';
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  /* Field: Flag friend: Message */
  $handler->display->display_options['fields']['message']['id'] = 'message';
  $handler->display->display_options['fields']['message']['table'] = 'flag_friend_message';
  $handler->display->display_options['fields']['message']['field'] = 'message';
  $handler->display->display_options['fields']['message']['relationship'] = 'flag_friend_content_rel';
  /* Field: Flags: Flag link */
  $handler->display->display_options['fields']['ops']['id'] = 'ops';
  $handler->display->display_options['fields']['ops']['table'] = 'flag_content';
  $handler->display->display_options['fields']['ops']['field'] = 'ops';
  $handler->display->display_options['fields']['ops']['relationship'] = 'flag_friend_content_rel';
  $handler->display->display_options['fields']['ops']['label'] = 'Actions';
  /* Field: Flag friend: Flag friend links */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'flag_friend';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: Flags: Content ID */
  $handler->display->display_options['arguments']['content_id']['id'] = 'content_id';
  $handler->display->display_options['arguments']['content_id']['table'] = 'flag_content';
  $handler->display->display_options['arguments']['content_id']['field'] = 'content_id';
  $handler->display->display_options['arguments']['content_id']['relationship'] = 'flag_friend_content_rel';
  $handler->display->display_options['arguments']['content_id']['default_action'] = 'default';
  $handler->display->display_options['arguments']['content_id']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['content_id']['default_argument_type'] = 'user';
  $handler->display->display_options['arguments']['content_id']['default_argument_options']['user'] = FALSE;
  $handler->display->display_options['arguments']['content_id']['validate_type'] = 'php';
  $handler->display->display_options['arguments']['content_id']['validate_options']['code'] = 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
                                        return FALSE;
                                      }
                                      return TRUE;';
  $handler->display->display_options['arguments']['content_id']['break_phrase'] = 0;
  $handler->display->display_options['arguments']['content_id']['not'] = 0;
  $handler->display->display_options['path'] = 'user/%/friends/pending';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Friend Requests';
  $handler->display->display_options['menu']['weight'] = '0';

  /* Display: Page (flagged) */
  $handler = $view->new_display('page', 'Page (flagged)', 'page_3');
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'Awaiting Friend Approvals';
  $handler->display->display_options['defaults']['header'] = FALSE;
  /* Header: Global: Text area */
  $handler->display->display_options['header']['text']['id'] = 'area';
  $handler->display->display_options['header']['text']['table'] = 'views';
  $handler->display->display_options['header']['text']['field'] = 'area';
  $handler->display->display_options['header']['text']['empty'] = TRUE;
  $handler->display->display_options['header']['text']['content'] = 'These are users who you have requested to be friends with.';
  $handler->display->display_options['header']['text']['format'] = '1';
  $handler->display->display_options['defaults']['empty'] = FALSE;
  /* Empty text: Global: Text area */
  $handler->display->display_options['empty']['text']['id'] = 'area';
  $handler->display->display_options['empty']['text']['table'] = 'views';
  $handler->display->display_options['empty']['text']['field'] = 'area';
  $handler->display->display_options['empty']['text']['empty'] = FALSE;
  $handler->display->display_options['empty']['text']['content'] = 'No Friend Requests.';
  $handler->display->display_options['defaults']['relationships'] = FALSE;
  /* Relationship: Flags: friend */
  $handler->display->display_options['relationships']['flag_content_rel']['id'] = 'flag_content_rel';
  $handler->display->display_options['relationships']['flag_content_rel']['table'] = 'users';
  $handler->display->display_options['relationships']['flag_content_rel']['field'] = 'flag_content_rel';
  $handler->display->display_options['relationships']['flag_content_rel']['flag'] = 'friend';
  $handler->display->display_options['relationships']['flag_content_rel']['user_scope'] = 'any';
  /* Relationship: Flags: User */
  $handler->display->display_options['relationships']['uid']['id'] = 'uid';
  $handler->display->display_options['relationships']['uid']['table'] = 'flag_content';
  $handler->display->display_options['relationships']['uid']['field'] = 'uid';
  $handler->display->display_options['relationships']['uid']['relationship'] = 'flag_content_rel';
  $handler->display->display_options['relationships']['uid']['required'] = 0;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: User: Picture */
  $handler->display->display_options['fields']['picture']['id'] = 'picture';
  $handler->display->display_options['fields']['picture']['table'] = 'users';
  $handler->display->display_options['fields']['picture']['field'] = 'picture';
  $handler->display->display_options['fields']['picture']['label'] = 'User';
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  /* Field: Flag friend: Message */
  $handler->display->display_options['fields']['message']['id'] = 'message';
  $handler->display->display_options['fields']['message']['table'] = 'flag_friend_message';
  $handler->display->display_options['fields']['message']['field'] = 'message';
  $handler->display->display_options['fields']['message']['relationship'] = 'flag_content_rel';
  /* Field: Flags: Flag link */
  $handler->display->display_options['fields']['ops']['id'] = 'ops';
  $handler->display->display_options['fields']['ops']['table'] = 'flag_content';
  $handler->display->display_options['fields']['ops']['field'] = 'ops';
  $handler->display->display_options['fields']['ops']['relationship'] = 'flag_content_rel';
  $handler->display->display_options['fields']['ops']['label'] = 'Actions';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: User: Uid */
  $handler->display->display_options['arguments']['uid']['id'] = 'uid';
  $handler->display->display_options['arguments']['uid']['table'] = 'users';
  $handler->display->display_options['arguments']['uid']['field'] = 'uid';
  $handler->display->display_options['arguments']['uid']['relationship'] = 'uid';
  $handler->display->display_options['arguments']['uid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['uid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['uid']['default_argument_type'] = 'user';
  $handler->display->display_options['arguments']['uid']['default_argument_options']['user'] = FALSE;
  $handler->display->display_options['arguments']['uid']['validate_type'] = 'php';
  $handler->display->display_options['arguments']['uid']['validate_options']['code'] = 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
      return FALSE;
    }
    return TRUE;';
  $handler->display->display_options['arguments']['uid']['break_phrase'] = 0;
  $handler->display->display_options['arguments']['uid']['not'] = 0;
  $handler->display->display_options['path'] = 'user/%/friends/flagged';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Awaiting Friend Approvals';
  $handler->display->display_options['menu']['weight'] = '0';

  /* Display: Current user's Friends block */
  $handler = $view->new_display('block', 'Current user\'s Friends block', 'block_1');
  $handler->display->display_options['defaults']['items_per_page'] = FALSE;
  $handler->display->display_options['defaults']['use_more'] = FALSE;
  $handler->display->display_options['use_more'] = TRUE;
  $handler->display->display_options['defaults']['relationships'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  /* Field: Flag friend: Flag friend links */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'flag_friend';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: Flag friend: Friends of */
  $handler->display->display_options['arguments']['friend_uid']['id'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['table'] = 'flag_friend';
  $handler->display->display_options['arguments']['friend_uid']['field'] = 'friend_uid';
  $handler->display->display_options['arguments']['friend_uid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['friend_uid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['friend_uid']['default_argument_type'] = 'current_user';
  $handler->display->display_options['arguments']['friend_uid']['break_phrase'] = 0;
  $handler->display->display_options['arguments']['friend_uid']['not'] = 0;
  $translatables['friends'] = array(
    t('Defaults'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort By'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('No Friends have been added.'),
    t('flag friend'),
    t('Name'),
    t('Flag link'),
    t('Flag friend links'),
    t('All'),
    t('Page (friends)'),
    t('Friends'),
    t('Friend'),
    t('Actions'),
    t('Page (pending)'),
    t('Friend Requests'),
    t('These are users who would like to be your friend.'),
    t('No Friend Requests.'),
    t('User'),
    t('Message'),
    t('Page (flagged)'),
    t('Awaiting Friend Approvals'),
    t('These are users who you have requested to be friends with.'),
    t('flag'),
    t('Flag user'),
    t('Current user\'s Friends block'),
  );
  $views[$view->name] = $view;
  return $views;
}
