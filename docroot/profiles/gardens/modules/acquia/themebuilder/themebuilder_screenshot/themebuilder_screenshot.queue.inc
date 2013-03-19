<?php
// $Id$

/**
 * @file
 * Queue handlers used by the themebuilder screenshot system.
 */

/**
 * Themebuilder screenshot queue handler.
 *
 * This handler behaves the same as the default SystemQueue, but it adds a
 * getAllItems() method that allows all current items in the queue to be
 * retrieved as an array.
 */
class ThemebuilderScreenshotQueue extends SystemQueue {
 /**
  * Retrieve all remaining items in the queue.
  *
  * This custom method is not part of the standard DrupalQueueInterface, but it
  * is similar to one that other queue implementations define (for example,
  * BatchQueue).
  *
  * @return
  *   An array of queue item objects, keyed by the item ID.
  */
  public function getAllItems() {
    $items = db_query('SELECT data, item_id FROM {queue} WHERE name = :name', array(':name' => $this->name))->fetchAllAssoc('item_id');
    foreach ($items as &$item) {
      $item->data = unserialize($item->data);
    }
    return $items;
  }
}
