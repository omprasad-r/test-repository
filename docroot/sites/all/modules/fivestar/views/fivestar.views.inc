<?php

/**
* Implements hook_views_data_alter().
*/
function fivestar_views_data_alter(&$data) {
  $data['votingapi_vote']['value']['field']['handler'] = 'fivestar_views_handler_field_value';
  $data['votingapi_cache']['value']['field']['handler'] = 'fivestar_views_handler_field_value';
}
