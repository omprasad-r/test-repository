<?php

class AlbumMigration extends WarnerMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the album type
    $specific_fields = array(
      'rating' => '',
      'release_date' => '',
      'author' => '',
      'track' => '',
      'secondary_title' => '',
      'copyright_name' => '',
      'copyright_url' => '',
      'buy_link_url' => '',
      'thumbnail' => '',
    );
    parent::__construct($specific_fields, 'album', 'album', $live);
    $this->description = t('Import album entities from XML into Drupal');
    $this->softDependencies = array('Audio');

    $this->addFieldMapping('field_album_date', 'release_date')
         ->xpath('release_date')
         ->callbacks(array($this, 'checkDate'));
    $this->addFieldMapping('field_recording_artist', 'author')
         ->xpath('author');
    $this->addFieldMapping('field_buy_links')
         ->description('constructed in prepare()');
    $this->addFieldMapping('field_tracks', 'track')
         ->xpath('track/@EosId')
         ->sourceMigration('Audio')
         ->arguments(array('format' => $this->safe_html));
    $this->addFieldMapping('field_album_tracks_num')
         ->description('Computed at run-time');
    $this->addFieldMapping('field_rating', 'rating')
         ->xpath('rating/rating_value');
    $this->addFieldMapping('field_secondary_title', 'secondary_title')
         ->xpath('secondary_title');
    $this->addFieldMapping('field_copyright_name', 'copyright_name')
         ->xpath('copyright_name');
    $this->addFieldMapping('field_copyright_url', 'copyright_url')
         ->xpath('copyright_url');
    $this->addFieldMapping('field_album_subtitle')
         ->issueGroup('DNM');
    $this->addFieldMapping('field_album_image', 'thumbnail')
         ->arguments(array('file_function' => 'file_copy', 'file_replace' => FILE_EXISTS_RENAME))
         ->xpath('thumbnail');
  }

  /**
   * Make sure an empty date is really empty.
   */
  protected function checkDate($date) {
    if (empty($date)) {
      $date = NULL;
    }
    return $date;
  }

  /**
   * Strip enclosing quotes from a URL.
   * @param $url
   * @return string
   */
  protected function processUrl($url) {
    return trim($url, '"');
  }

  public function prepare($node, $row) {
    // Until field_tracks is a real node reference, rewrite to an explicit link
    $count = 0;
    if (isset($node->field_tracks) && is_array($node->field_tracks) && !empty($node->field_tracks)) {
      foreach ($node->field_tracks[LANGUAGE_NONE] as $delta => $instance) {
        $audio_nid = $instance['value'];
        $title = db_select('node', 'n')
                 ->fields('n', array('title'))
                 ->condition('nid', $audio_nid)
                 ->execute()
                 ->fetchField();
        $node->field_tracks[LANGUAGE_NONE][$delta]['value'] =
          l($title, "node/$audio_nid");
        $count++;
      }
    }
    $node->field_album_tracks_num[LANGUAGE_NONE][0]['value'] = $count;

    // Fix up buy URLs. This is necessary because the dumps create a duplicate of
    // the album for each distinct buy_link_url value. We use our XML source to
    // pull items with the same EosId as ourselves, and grab the link urls and names.
    $eos_id = $row->EosId;
    $item_xpath = "/file/article[@EosId='$eos_id']";  // Select elements of the requested type
    $item_ID_xpath = "buy_link_url";
    $source = new MigrateSourceXML($this->items_url, $item_xpath, $item_ID_xpath);
    foreach ($source as $url_row) {
      $url = $this->processUrl((string)$url_row->xml->buy_link_url);
      if ($url) {
        $name = $this->processUrl((string)$url_row->xml->buy_link_name);
        $link = "<a href='$url'>$name</a>";
        $link = drupal_substr($link, 0, 255);
        $node->field_buy_links[LANGUAGE_NONE][] = array(
          'format' => $this->safe_html,
          'value_format' => $this->safe_html,
          'value' => $link,
        );
      }
    }
  }

  public function complete($node, $row) {
    $this->handleRating($node, $row);
  }
}

class AlbumNonLiveMigration extends AlbumMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live album entities from XML into Drupal');
  }
}
