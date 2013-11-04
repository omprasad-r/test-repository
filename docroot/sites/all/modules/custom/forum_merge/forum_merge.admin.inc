<?php
/**
 * @file
 * Admin pages to merge or revert forums.
 *
 */

/**
 * Form builder to forum merge page.
 *
 * @see forum_merge_menu().
 */
function forum_merge_page_form($form, $form_state, $nid) {
  $form['#nid'] = $nid;

  $form['forum'] = array(
    '#type' => 'textfield',
    '#title' => t('Topic title'),
    '#required' => TRUE,
    '#description' => t('Choose forum topic which you would like to merge into this topic.'),
    '#autocomplete_path' => 'forum_merge/autocomplete/',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Merge'),
  );

  return $form;
}

/**
 * Validate handler for forum_merge_page_form().
 */
function forum_merge_page_form_validate($form, &$form_state) {
  if ($form_state['values']['forum']) {
    $form_state['forum_id'] = 0;
    $forum_id = forum_merge_parse_id($form_state['values']['forum']);
    // If just the title has been given.
    if (!$forum_id) {
      // Get the topics that satisfy the conditions.
      $query = db_select('node', 'n');
      $query->addTag('node_access');

      // Select rows that match by node name.
      $result = $query
        ->fields('n', array('nid', 'title'))
        ->condition('n.title', $form_state['values']['forum'])
        ->execute()
        ->fetchAll();

      // If get results based on the given title.
      if ($result) {
        // More than one results.
        if (count($result) > 1) {
          $message = 'There are more than one title of forum topics are matched to the following: %title';
          form_set_error('forum', t($message, array('%title' => $form_state['values']['forum'])));
        }
        $result = reset($result);
        $forum_id = $result->nid;
      }
      else {
        // No result.
        $message = 'There is no forum topic named: %title';
        form_set_error('forum', t($message, array('%title' => $form_state['values']['forum'])));
      }
    }
    if ($forum_id) {
      // If one of the given topic is the topic that the user wants to merge into.
      if ($forum_id === $form['#nid']) {
        $message = 'You can\'t merge a topic into itself!';
        form_set_error('forum', t($message));
      }

      // Check the node and the node access.
      $node = node_load($forum_id);
      if (!$node) {
        $message = 'The topic with id %nid doesn\'t exist.';
        form_set_error('forum', t( $message, array('%nid' => $forum_id)));
      }
      elseif (!node_access('delete', $node)) {
        $message = 'You have no rights to be able to delete the forum topic with the following title: %title';
        form_set_error('forum', t( $message, array('%title' => $node->title)));
      }
      $form_state['forum_id'] = $forum_id;
    }
  }
}

/**
 * Submit handler for forum_merge_page_form().
 */
function forum_merge_page_form_submit($form, $form_state) {
  $base_node = node_load($form['#nid']);
  $forum_node = node_load($form_state['forum_id']);

  $batch = array(
    'title' => t('Merging...'),
    'operations' => array(
      array('forum_merge_save_merged_comments', array($base_node, $forum_node)),
      array('forum_merge_rebuild_threads', array($base_node))
    ),
    'finished' => 'forum_merge_finish_merge_callback',
  );

  batch_set($batch);
}

/**
 * Form builder to forum merge page.
 *
 * @see forum_merge_menu().
 */
function forum_merge_revert_form($form, $form_state, $nid) {
  // Get base (reverting) node.
  $node = node_load($nid);
  $form['#node'] = $node;

  return confirm_form($form, t('Do you really want to revert %name forum topic?', array('%name' => $node->title)), 'node/' . $node->nid);
}

/**
 * Submit handler for forum_merge_revert_form().
 */
function forum_merge_revert_form_submit($form, &$form_state) {
  $reverted_node = $form['#node'];
  // Set the redirect to the node page.
  $form_state['redirect'] = 'node/' . $reverted_node->nid;

  $batch = array(
    'title' => t('Merging...'),
    'operations' => array(
      array('forum_merge_save_reverted_comments', array($reverted_node)),
      array('forum_merge_rebuild_threads', array($reverted_node))
    ),
    'finished' => 'forum_merge_finish_revert_callback',
  );

  batch_set($batch);
}
