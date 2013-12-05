<?php

/**
 * @file
 * Contains AcsfDuplicationScrubCommentHandler.
 */

/**
 * Handles the scrubbing of Drupal comments.
 */
class AcsfDuplicationScrubCommentHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];
    $comment_batch = $options['comment_batch'] ? $options['comment_batch'] : 1000;

    if ($options['retain_content'] || !module_exists('comment')) {
      return;
    }

    if ($options['avoid_oom']) {
      // Orphaned comments, that is comments by an authenticated user that no
      // longer exists, cannot be deleted by comment_delete_multiple. Handle
      // these items first.
      if ($cids = self::getOrphanedItems($comment_batch)) {
        $orphaned = TRUE;
      }
      elseif ($cids = self::getItems($comment_batch)) {
        $orphaned = FALSE;
      }

      if (!empty($cids)) {
        self::deleteItems($cids, $orphaned);
        $this->event->dispatcher->interrupt();
      }
    }
    else {
      do {
        if ($cids = self::getOrphanedItems($comment_batch)) {
          $orphaned = TRUE;
        }
        elseif ($cids = self::getItems($comment_batch)) {
          $orphaned = FALSE;
        }
        else {
          break;
        }

        self::deleteItems($cids, $orphaned);
      } while (TRUE);
    }

  }

  /**
   * Counts the remaining comments.
   *
   * @return int
   *   The number of items remaining in the table.
   */
  public function countRemaining() {
    return db_query('SELECT COUNT(*) FROM {comment}')->fetchField();
  }

  /**
   * Gets a range of comment IDs.
   *
   * @param int $limit
   *   The number of records to retrieve.
   *
   * @return array
   *   An indexed array containing the relevant comment IDs, or an empty array
   *   if there is no result set.
   */
  protected function getItems($limit) {
    return db_query_range('SELECT cid FROM {comment}', 0, $limit)->fetchCol();
  }

  /**
   * Gets a range of orphaned comment IDs.
   *
   * Orphaned comments are those which were authored by an authenticated user
   * that for some reason no longer exist on the site.
   *
   * @param int $limit
   *   The number of records to retrieve.
   *
   * @return array
   *   An indexed array containing the relevant comment IDs, or an empty array
   *   if there is no result set.
   */
  protected function getOrphanedItems($limit) {
    return db_query_range('SELECT cid FROM {comment} c LEFT JOIN {users} u ON c.uid = u.uid WHERE u.uid IS NULL', 0, $limit)->fetchCol();
  }

  /**
   * Deletes comments.
   *
   * @param array $cids
   *   An indexed array of comment IDs to delete.
   * @param bool $orphaned
   *   Optional. Whether or not the comments are orphans, since these need to be
   *   deleting differently.
   */
  protected function deleteItems($cids, $orphaned = FALSE) {
    if ($orphaned) {
      db_delete('comment')->condition('cid', $cids, 'IN')->execute();
    }
    else {
      comment_delete_multiple($cids);
    }
  }

}

