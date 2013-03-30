<?php

/**
 * Form for bulk adding existing nodes to the original_author table.
 */
function original_author_bulk_add() {
  $form = array();
  $form['bulk_add'] = array(
    '#type' => 'markup',
    '#markup' => '<div>' . t('Clicking the button below will cause all existing nodes to be added to the original_author table. The uid set for each one will be the uid of the first revision of that node.') . '</div>',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Bulk add nodes'),
  );
  return $form;
}

/**
 * Submit function for the bulk add form.
 */
function original_author_bulk_add_submit($form, &$form_state) {
  $queue = DrupalQueue::get('original_author_queue');
  try {
    $result = db_query('SELECT n.nid FROM {node} n LEFT JOIN {original_author} a ON n.nid = a.nid WHERE a.nid IS NULL');
    foreach ($result as $row) {
      $item = array(
        'nid' => $row->nid,
      );
      $queue->createItem($item);
    }
  }
  catch (Exception $e) {
    watchdog_exception('original_author_bulk', $e);
  }
  drupal_set_message(t('All nodes have been queued for adding to the original_author table.'));
}