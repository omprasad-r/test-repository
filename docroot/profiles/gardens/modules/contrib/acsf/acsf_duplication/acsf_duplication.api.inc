<?php

/**
 * @file
 * Documents hooks provided by the ACSF Duplication module.
 */

/**
 * Alters the counts reported by `drush acsf-duplication-scrub-progress`.
 *
 * @param $data
 *   An associative array of counts representing the total number of items
 *   remaining to scrub, keyed by [type]_count.
 *
 * @see drush_acsf_duplication_scrub_progress()
 */
function hook_acsf_duplication_scrub_remaining_counts_alter(&$data) {
  $data['node_count'] = AcsfDuplicationScrubNodeHandler::countRemaining();
}

/**
 * Alters the scrub event context of `drush acsf-duplication-scrub-batch`.
 *
 * Use this alter hook to add optional data to the scrub event. The data added
 * here is available via the $this->event->context array in event handlers.
 *
 * @param $data
 *   An associative array of context data needed in the event handlers.
 *
 * @see drush_acsf_duplication_scrub_batch()
 */
function hook_acsf_duplication_scrub_context_alter(&$data) {
  $data['scrub_options']['retain_users'] = drush_get_option('exact-copy');
  $data['scrub_options']['retain_content'] = drush_get_option('exact-copy');
}

