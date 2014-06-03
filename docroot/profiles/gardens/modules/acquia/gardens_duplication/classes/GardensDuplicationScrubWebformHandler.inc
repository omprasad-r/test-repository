<?php

/**
 * @file
 * Contains GardensDuplicationScrubWebformHandler.
 */

/**
 * Scrubs Webform submissions.
 *
 * This handler deletes Webform submissions in batches but does not delete the
 * webform nodes themselves. The assumption is that the duplicated site should
 * have the same webforms as the source site.
 */
class GardensDuplicationScrubWebformHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];
    $limit = $options['batch_webform'];

    if ($options['retain_webform_submissions'] || !module_exists('webform')) {
      return;
    }

    module_load_include('inc', 'webform', 'includes/webform.submissions');

    if ($options['avoid_oom']) {
      if ($submissions = self::getItems($limit)) {
        self::deleteItems($submissions);
        $this->event->dispatcher->interrupt();
      }
    }
    else {
      do {
        $submissions = self::getItems($limit);
        if (empty($submissions)) {
          break;
        }
        self::deleteItems($submissions);
      } while (TRUE);
    }
  }

  /**
   * Counts the remaining webform submissions.
   *
   * @return int
   *   The number of items remaining in the webform_submissions table.
   */
  public function countRemaining() {
    return db_query('SELECT COUNT(*) FROM {webform_submissions}')->fetchField();
  }

  /**
   * Gets a range of webform submissions.
   *
   * @param int $limit
   *   The number of records to retrieve.
   *
   * @return array
   *   An array of webform submissions keyed by NID.
   */
  public function getItems($limit) {
    module_load_include('inc', 'webform', 'includes/webform.submissions');
    return webform_get_submissions(array(), NULL, $limit);
  }

  /**
   * Deletes webform submissions.
   *
   * @param array $submissions
   *   An array of webform submissions to delete.
   */
  protected function deleteItems($submissions) {
    // Make a list of NIDs from the submissions and load the respective nodes,
    // which are required for deleting submissions.
    $nids = array();
    foreach ($submissions as $submission) {
      $nids[$submission->nid] = $submission->nid;
    }
    $nodes = node_load_multiple($nids);

    // Delete submissions.
    foreach ($submissions as $submission) {
      $nid = $submission->nid;
      if (!empty($nodes[$nid]) && isset($nodes[$nid]->webform['components'])) {
        webform_submission_delete($nodes[$nid], $submission);
      }
      else {
        db_delete('webform')->condition('nid', $nid)->execute();
        db_delete('webform_component')->condition('nid', $nid)->execute();
        db_delete('webform_emails')->condition('nid', $nid)->execute();
        db_delete('webform_roles')->condition('nid', $nid)->execute();
        db_delete('webform_submissions')->condition('nid', $nid)->execute();
        db_delete('webform_submitted_data')->condition('nid', $nid)->execute();
        db_delete('webform_last_download')->condition('nid', $nid)->execute();
      }
    }
  }

}

