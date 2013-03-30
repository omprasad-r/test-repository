<?php
/**
 * @file
 * Release migration class for UMG.
 */

/**
 * Base migration class for the discography JSON migrations.
 */
abstract class UMGDiscographyMigration extends UMGMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $connection = Database::getConnection('default', 'umg_legacy');
    $result = $connection->select('artists', 'a')
              ->fields('a', array('FName', 'LName'))
              ->condition('ArtistID', $arguments['artist_id'])
              ->execute();

    $data = $result->fetchAssoc();
    $first = trim($data['FName']);
    $last = trim($data['LName']);
    if ($first) {
      $this->artistName = $first . ' ' . $last;
    }
    else {
      $this->artistName = $last;
    }
  }

  /**
   * Generate the appropriate discography services URL for the given artist
   * name.
   *
   * @param $artist_name
   *
   * @return string
   */
  static public function getDiscographyUrl($artist_name) {
    $artist_name = trim($artist_name);
    // Overrides for artists with discrepencies between their names in the
    // database and their names in the fields.
    $artist_name_overrides = array(
      '2 Pac' => '2Pac',
    );
    if (isset($artist_name_overrides[$artist_name])) {
      $artist_name = $artist_name_overrides[$artist_name];
    }

    $service_url = 'http://dev-cmt.umgwebservices.com/dal/releases.json?artist='
      . urlencode($artist_name) . '&tracks=True';
    return $service_url;
  }
}

/**
 * Release migration class for UMG.
 */
class UMGReleaseMigration extends UMGDiscographyMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $fields = array(
      // Release fields
      'id' => 'Unique release ID',
      'name' => 'Name of the release',
      'release_date' => 'Date of release',
      'label_id' => 'Reference to record label',
      'disc_count' => 'Number of physical discs in the release',
      'track_count' => 'Number of tracks in the release',
      'album_art' => 'URL of the album cover (tif version)',
      'format' => 'Release format - Physical/Digital',
      'config_id' => 'Reference to type of release',
      'upc' => 'UPC code',
      'explicit' => 'Boolean explicit flag',
      'artist_id' => 'Reference to artist record',
      'album_art_jpg' => 'URL of the album cover (jpg version)',
      // Values pulled from related objects
      'label' => 'Name of record label',
      'config' => 'Type of release - E Single, Full Length Album, etc.',
      'artist' => 'Name of artist',
    );

    $this->source = new MigrateSourceJSON(self::getDiscographyUrl($this->artistName),
      'Release', $fields, array('reader_class' => 'UMGReleaseJSONReader',
        'cache_counts' => TRUE, 'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationNode('music');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Release ID',
                       )
        ),
        MigrateDestinationNode::getKeySchema()
      );

    $this->addFieldMapping('title', 'name');
    $this->addFieldMapping('uid')
         ->defaultValue($arguments['default_author_id']);
    $this->addFieldMapping('created', 'release_date');
    $this->addFieldMapping('changed', 'release_date');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_release_date', 'release_date');
    $this->addFieldMapping('field_upc', 'upc');
    $this->addFieldMapping('field_disc_count', 'disc_count');
    $this->addFieldMapping('field_track_count', 'track_count');
    $this->addFieldMapping('field_explicit', 'explicit')
         ->callbacks(array($this, 'handleExplicit'));
    $this->addFieldMapping('field_album_type', 'config');
    $this->addFieldMapping('field_album_type:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_album_category', 'format');
    $this->addFieldMapping('field_album_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_artist', 'artist');
    $this->addFieldMapping('field_artist:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_record_label', 'label');
    $this->addFieldMapping('field_record_label:create_term')
         ->defaultValue(TRUE);
    // Import is happier when the capitalized domain is lowercase.
    $this->addFieldMapping('field_album_cover', 'album_art_jpg')
         ->callbacks('strtolower');
    $this->addFieldMapping('field_album_cover:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_album_cover:alt', 'name');
    $this->addFieldMapping('field_album_cover:title', 'name');
    $this->addFieldMapping('field_track_collection')
         ->description('Handled by separate Track migration');

    $this->addUnmigratedDestinations(array(
        'field_album_category:source_type', 'field_album_category:ignore_case',
        'field_album_cover:file_class', 'field_album_cover:language',
        'field_album_cover:destination_dir', 'field_album_cover:destination_file',
        'field_album_cover:file_replace', 'field_album_cover:source_dir',
        'field_album_type:source_type', 'field_album_type:ignore_case',
        'field_artist:source_type', 'field_artist:ignore_case',
        'field_record_label:source_type', 'field_record_label:ignore_case',
        'field_release_date:timezone', 'field_release_date:rrule', 'field_release_date:to',
        'field_upc:language',
      'comment',
      'field_amazon_link',
      'field_itunes_link',
      'is_new',
      'language',
      'log',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'tnid',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'album_art',
      'artist_id',
      'config_id',
      'label_id',
    ));
  }

  /**
   * Translate JSON boolean (with empty FALSE value) to 1/0.
   *
   * @param $value
   *
   * @return int
   */
  protected function handleExplicit($value) {
    if ($value == 1) {
      return 1;
    }
    else {
      return 0;
    }
  }
}

/**
 * A JSON reader class that knows how to find the release data.
 */
class UMGReleaseJSONReader extends MigrateJSONReader {
  /**
   * Implements MigrateJSONReader::next().
   */
  public function next() {
    // Do the default fetch.
    parent::next();
    // Now, move the Release fields to the top level
    if (is_object($this->currentElement)) {
      foreach ($this->currentElement->Release as $field_name => $field_value) {
        $this->currentElement->$field_name = $field_value;
      }
      $this->currentId = $this->currentElement->id;
      unset($this->currentElement->Release);
      // Pull values from other objects
      $this->currentElement->label = $this->currentElement->Label->name;
      unset($this->currentElement->Label);
      $this->currentElement->config = $this->currentElement->Config->name;
      unset($this->currentElement->Config);
      $this->currentElement->artist = $this->currentElement->Artist->name;
      unset($this->currentElement->Artist);
    }
  }
}

/**
 * Track migration class for UMG.
 */
class UMGTrackMigration extends UMGDiscographyMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $release_migration = $this->group->getName() . 'Release';
    $this->dependencies[] = $release_migration;

    $fields = array(
      // Track fields
      'id' => 'Unique track ID',
      'name' => 'Name of the track',
      'release_id' => 'ID of the release containing this track',
      'disc_number' => 'Number of the disc holding this track',
      'track_number' => 'Number of the track within the disc',
      'duration' => 'Track duration in seconds',
      'isrc' => 'ISRC code',
    );

    $this->source = new MigrateSourceJSON(self::getDiscographyUrl($this->artistName),
      'Track', $fields, array('reader_class' => 'UMGTrackJSONReader', 'cache_counts' => TRUE, 'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationFieldCollection('field_track_collection',
      array('host_entity_type' => 'node'));

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Track ID',
                       )
        ),
        MigrateDestinationFieldCollection::getKeySchema()
      );

    $this->addFieldMapping('host_entity_id', 'release_id')
         ->sourceMigration($release_migration);
    $this->addFieldMapping('field_track_title', 'name');
    $this->addFieldMapping('field_track_number', 'track_number');
    $this->addFieldMapping('field_disc_number', 'disc_number');
    $this->addFieldMapping('field_duration', 'duration');
    $this->addFieldMapping('field_isrc', 'isrc');

    $this->addUnmigratedDestinations(array(
        'field_isrc:language',
        'field_track_title:language',
      'path',
    ));
  }
}

/**
 * A JSON reader class that knows how to find the track data.
 */
class UMGTrackJSONReader extends MigrateJSONReader {
  /**
   * The tracks we are currently iterating over.
   *
   * @var array
   */
  protected $cachedTracks = array();

  /**
   * Unique ID of the release that the tracks belong to.
   *
   * @var string
   */
  protected $releaseId = '';

  /**
   * Implements MigrateJSONReader::next().
   */
  public function next() {
    $this->currentElement = $this->currentId = NULL;
    if (empty($this->cachedTracks)) {
      // We have no tracks to work with, find the next parent object with
      // comments.
      parent::next();
      while (!empty($this->currentElement)) {
        if (isset($this->currentElement->Track[0])) {
          // We found some tracks - remember them and escape to process them.
          $this->cachedTracks = $this->currentElement->Track;
          $this->releaseId = $this->currentElement->Track[0]->release_id;
          break;
        }
        // Keep looking for a parent object with tracks.
        parent::next();
      }
    }
    // If we have tracks to work with, pull the first one off the queue.
    if (!empty($this->cachedTracks)) {
      $this->currentElement = array_shift($this->cachedTracks);
      $this->currentElement->release_id = $this->releaseId;
      $this->currentID = $this->currentElement->id;
    }
  }
}
