<?php

/**
 * @file
 * Contains GardensDuplicationScrubVotingApiHandler.
 */

/**
 * Scrubs the Voting API data, but preserves its configuration.
 */
class GardensDuplicationScrubVotingApiHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];
    $limit = $options['batch_votingapi'];

    if ($options['retain_voting_api_data'] || !module_exists('votingapi')) {
      return;
    }

    if ($options['avoid_oom']) {
      if ($results = self::getResults($limit)) {
        self::deleteResults($results);
      }
      elseif ($votes = self::getVotes($limit)) {
        self::deleteVotes($votes);
      }
      $this->event->dispatcher->interrupt();
    }
    else {
      do {
        if ($results = self::getResults($limit)) {
          self::deleteResults($results);
        }
        elseif ($votes = self::getVotes($limit)) {
          self::deleteVotes($votes);
        }
        else {
          break;
        }
      } while (TRUE);
    }

    variable_del('votingapi_last_cron');
  }

  public function getVotes($limit) {
    return votingapi_select_votes(array(), $limit);
  }

  public function getResults($limit) {
    return votingapi_select_results(array(), $limit);
  }

  public function deleteVotes($votes) {
    votingapi_delete_votes($votes);
  }

  public function deleteResults($results) {
    votingapi_delete_results($results);
  }
}

