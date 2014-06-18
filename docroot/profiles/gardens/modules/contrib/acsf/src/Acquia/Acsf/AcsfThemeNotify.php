<?php

/**
 * @file
 * Contains \Acquia\Acsf\AcsfThemeNotify.
 */

namespace Acquia\Acsf;

class AcsfThemeNotify {

  /**
   * Sends a theme notification to the Factory.
   *
   * This is going to contact the Factory, so it qualifies as a third-party
   * call, therefore calling it during a normal page load is not advisable. A
   * possibly safer solution could be executing this via a menu callback called
   * through an asynchronous JavaScript call.
   *
   * If the request does not succeed, the notification will be stored so that
   * we may try again later when cron runs.
   *
   * @param string $event
   *   The type of theme event that occurred.
   * @param string $theme
   *   The system name of the theme the event relates to. For site scoped theme
   *   notifications this variable may be empty.
   * @param int $timestamp
   *   The timestamp when the notification was created.
   * @param bool $store_failed_notification
   *   Optional variable to disable storing a notification when the sending
   *   fails. Should be only used in case of notifications which have been
   *   already added to the pending notification table.
   *
   * @return array
   *   The message response body and code.
   */
  public function sendNotification($event, $theme = '', $timestamp = NULL, $store_failed_notification = TRUE) {
    if (!$this->isEnabled()) {
      return array(
        'code' => 500,
        'data' => array('message' => t('The theme change notification feature is not enabled.')),
      );
    }

    try {
      $site = acsf_get_acsf_site();
      $nid = $site->site_id;
      $parameters = array('event' => $event);
      if ($timestamp) {
        $parameters['timestamp'] = $timestamp;
      }
      if ($theme) {
        $parameters['theme'] = $theme;
        $scope = 'theme';
      }
      else {
        $scope = 'site';
      }
      $message = new AcsfMessageRest('POST', "site-api/v1/theme/notification/$nid/$scope", $parameters);
      $message->send();
      $response = array(
        'code' => $message->getResponseCode(),
        'data' => $message->getResponseBody(),
      );
    }
    catch (AcsfMessageFailedResponseException $e) {
      $error_message = t('AcsfThemeNotify failed with error: @message', array('@message' => $e->getMessage()));
      syslog(LOG_ERR, $error_message);
      $response = array(
        'code' => 500,
        'data' => array('message' => $error_message),
      );
    }

    if ($store_failed_notification && $response['code'] !== 200) {
      $this->addNotification($event, $theme);
    }

    return $response;
  }

  /**
   * Resends failed theme notifications.
   *
   * @param int $limit
   *   The number of notification that should be processed.
   *
   * @return int
   *   Returns the number of successfully sent notifications. If none of the
   *   pending notifications managed to get sent then the return will be -1.
   */
  public function processNotifications($limit) {
    if (!$this->isEnabled()) {
      return -1;
    }

    $notifications = $this->getNotifications($limit);

    // If there were no pending notifications then we can consider this process
    // successful.
    $success = 0;

    foreach ($notifications as $notification) {
      // If this is a notification for an event that is not supported, it will
      // never get a 200 response so we need to remove it from storage.
      if (!in_array($notification->event, array('create', 'modify', 'delete'))) {
        $this->removeNotification($notification);
        continue;
      }
      // Try to send the notification but if it fails do not store it again.
      $response = $this->sendNotification($notification->event, $notification->theme, $notification->timestamp, FALSE);
      if ($response['code'] === 200) {
        $this->removeNotification($notification);
        $success++;
      }
    }

    return $success == 0 && !empty($notifications) ? -1 : $success;
  }

  /**
   * Removes a pending notification from the database.
   */
  public function removeNotification($notification) {
    db_query('DELETE FROM {acsf_theme_notifications} WHERE id = :id', array(
      ':id' => $notification->id,
    ));
  }

  /**
   * Indicates whether theme notifications are enabled.
   *
   * If this method returns FALSE, theme notifications will not be sent to the
   * Site Factory.
   *
   * @return bool
   *   TRUE if notifications are enabled; FALSE otherwise.
   */
  public function isEnabled() {
    return acsf_vget('acsf_theme_enabled', TRUE);
  }

  /**
   * Gets a list of stored notifications to be resent.
   *
   * @param int $limit
   *   The number of notifications to fetch.
   *
   * @return object[]
   *   An array of theme notification objects.
   */
  public function getNotifications($limit) {
    return db_query_range('SELECT id, event, theme, timestamp FROM {acsf_theme_notifications} ORDER BY timestamp ASC', 0, $limit)->fetchAll();
  }

  /**
   * Stores a theme notification for resending later.
   *
   * If the initial request to send the notification to the Factory fails, we
   * store it and retry later on cron.
   *
   * @param string $event
   *   The type of theme event that occurred.
   * @param string $theme
   *   The system name of the theme the event relates to.
   */
  public function addNotification($event, $theme) {
    db_query('INSERT INTO {acsf_theme_notifications} (timestamp, event, theme) VALUES(:timestamp, :event, :theme)', array(
      ':timestamp' => time(),
      ':event' => $event,
      ':theme' => $theme,
    ));
  }

}
