<?php
/**
 * @file
 * Holds most of the code to handle custom added prepareRow transforms.
 */

/**
 * Class CrateD2DMigrateHandler
 */
class CrateD2DMigrateHandler {

  protected $migration;
  protected $currentRow = NULL;
  protected static $instance;

  /**
   * Constructor.
   *
   * @param Migration $migration
   *   Holds the source migration to work from.
   */
  public function __construct(Migration $migration) {
    $this->migration = $migration;
  }

  /**
   * Magic __call method to prevent errors if the ui passes a bad function.
   *
   * @param string $method
   *   Method name.
   * @param array $args
   *   Method arguments.
   */
  public function __call($method, $args) {
    // Alert or something?
  }

  /**
   * Returns any available transforms.
   *
   * @return array
   *   Transforms by kv-pairs.
   */
  public static function getAvailableModifiers() {
    return array(
      'unserializeDate'   => 'Unserialize Date',
      'nullOnEmpty'       => 'Null On Empty',
      'replaceEmbedCodes' => 'Fix Youtube Embed Code',
      'getRatingRaw'      => 'Get Raw Rating from VotingAPI',
      'getRating'         => 'Get 5-star Rating from VotingAPI',
    );
  }

  /**
   * Singleton method.
   *
   * @param MigrationBase $migration
   *   Target Migration.
   *
   * @return CrateD2DMigrateHandler
   *   Returns a new copy of itself.
   */
  public static function getInstance(MigrationBase $migration) {
    if (!isset(static::$instance)) {
      static::$instance = new static($migration);
    }

    return static::$instance;
  }

  /**
   * Loads field modifiers from the database.
   *
   * @return array
   *   Field modifiers.
   */
  public function getFieldModifiers() {
    $query = db_select('crate_d2d_field_extras', 'fe')
      ->fields('fe')
      ->condition('migration_name', $this->migration->getMachineName());

    $results = $query->execute()->fetchAll();
    $return = array();

    foreach ($results as $row) {
      if (!isset($return[$row->field_name])) {
        $return[$row->field_name] = array();
      }

      $return[$row->field_name][] = $row->callback;
    }

    return $return;
  }

  /**
   * Saves a new field modifier to the database.
   *
   * @param string $field
   *   The field name.
   * @param array|string $modifiers
   *   The machine name of a modifier.
   *
   * @return self
   *   Self for chaining.
   */
  public function addFieldModifiers($field, $modifiers) {
    if (empty($modifiers)) {
      return $this;
    }

    if (!is_array($modifiers)) {
      $modifiers = array($modifiers);
    }

    $query = db_insert('crate_d2d_field_extras');
    $query->fields(array('migration_name', 'field_name', 'callback'));

    foreach ($modifiers as $mod) {
      $query->values(array(
        'migration_name' => $this->migration->getMachineName(),
        'field_name'     => $field,
        'callback'       => $mod,
      ));
    }

    $query->execute();

    return $this;
  }

  /**
   * Removes a list of field modifiers from the database.
   *
   * @param string $field
   *   The field name.
   * @param array|string $modifiers
   *   The machine name of a modifier.
   *
   * @return self
   *   Self for chaining.
   */
  public function removeFieldModifiers($field, $modifiers) {
    if (empty($modifiers)) {
      return $this;
    }

    if (!is_array($modifiers)) {
      $modifiers = array($modifiers);
    }

    db_delete('crate_d2d_field_extras')
      ->condition('field_name', $field)
      ->condition('callback', $modifiers, 'IN')
      ->execute();

    return $this;
  }

  /**
   * Called by prepareRow to actually run the transforms.
   *
   * @param StdClass $row
   *   Migraiton source row.
   */
  public function runFieldModifiers($row) {
    $mods = $this->getFieldModifiers();

    $this->currentRow = $row;

    foreach ($mods as $field_name => $callbacks) {
      foreach ($callbacks as $callback) {
        $row->{$field_name} = $this->{$callback}($row->{$field_name});
      }
    }
  }

  /**
   * Intended to track a node relationship that doesn't exist.
   *
   * For example, this is the album/track thing.
   *
   * @todo Make the source table configurable?
   *
   * @param string $value
   *   The source value, which doesn't matter to this method.
   *
   * @return NULL|int
   *   The source id or NULL
   */
  protected function reverseTrackRelationship($value) {
    $migrate_arguments = $this->migration->getArguments();

    // Find the source ID of the attached node:
    $source_nid = Database::getConnection('default', $migrate_arguments['source_connection'])
      ->select('content_field_album_tracks', 't')
      ->fields('t', array('nid'))
      ->condition('field_album_tracks_nid', $this->currentRow->nid)
      ->execute()
      ->fetchField();

    if (empty($source_nid)) {
      return NULL;
    }

    // Just return the source id, because you can set the source migration as appropriate.
    return $source_nid;
  }

  /**
   * Unserializes a drupal 5 profile date.
   *
   * @param string $value
   *   Source value.
   */
  protected function unserializeDate($value) {
    $val = unserialize($value);

    if (empty($val)) {
      return $value;
    }

    $month = str_pad($val['month'], 2, '0', STR_PAD_LEFT);
    $day = str_pad($val['day'], 2, '0', STR_PAD_LEFT);

    return "{$val['year']}-{$month}-{$day}";
  }

  /**
   * Returns NULL if the value evaluates to empty.
   *
   * @param mixed $value
   *   Value from $row
   *
   * @return null|mixed
   *   Either NULL or the original value.
   */
  protected function nullOnEmpty($value) {
    if (empty($value)) {
      return NULL;
    }

    return $value;
  }

  /**
   * Gets the 5-star normalized version of the rating.
   *
   * @param string $value
   *   Rating value (probably NULL or 0).
   *
   * @return float|int
   *   Parsed rating.
   */
  protected function getRating($value) {
    $rating_info = $this->getRatingInfo();

    if (empty($rating_info)) {
      return 0;
    }

    if ($rating_info->value_type == 'percent') {
      return $rating_info->value / 20;
    }

    return $rating_info->value;
  }

  /**
   * Gets the raw rating out of votingapi's cache.
   *
   * @param string $value
   *   Original value.
   *
   * @return int
   *   Rating.
   */
  protected function getRatingRaw($value) {
    $rating_info = $this->getRatingInfo();

    if (empty($rating_info)) {
      return 0;
    }

    return $rating_info->value;
  }

  /**
   * Queries the source database for rating information.
   *
   * @return mixed
   *   Database record. Probably StdClass (or Cursor), or NULL.
   */
  protected function getRatingInfo() {
    $migrate_arguments = $this->migration->getArguments();

    $rating_q = Database::getConnection('default', $migrate_arguments['source_connection'])
      ->select('votingapi_cache', 'v')
      ->fields('v', array('value', 'value_type'))
      ->condition('content_id', $this->currentRow->nid)
      ->condition('content_type', 'node')
      ->condition('function', 'average');

    $rating_info = $rating_q->execute()->fetch();

    return $rating_info;
  }

  /**
   * Replaces bad embed code data with just the youtube url (intended use).
   *
   * @param string $value
   *   Original Value.
   *
   * @return string
   *   Either replaced value or the original value.
   */
  protected function replaceEmbedCodes($value) {
    if (strpos($value, 'object') === FALSE) {
      return $value;
    }

    $embed_regex = '#<param name="movie" value=["\'](.*?)["\']\s*>#';

    preg_match($embed_regex, $value, $match);

    if (!empty($matches[1])) {
      $found_match = $matches[1];
    }

    if (empty($found_match)) {
      $iframe_regex = '#<iframe .*? src=["\'](.*?)["\'].*?>#';

      preg_match($iframe_regex, $value, $matches);

      if (!empty($matches[1])) {
        $found_match = $matches[1];
      }
    }

    if (empty($found_match)) {
      return $value;
    }

    // Deal with weird oembed links
    if (strpos($found_match, 'http://www.youtube.com/oembed?iframe=0&wmode=transparent&url=') !== FALSE) {
      $found_match = urldecode(str_replace('http://www.youtube.com/oembed?iframe=0&wmode=transparent&url=', '', $found_match));
    }

    // Handle // uris.
    if ($found_match == '/') {
      $found_match = "http:" . $found_match;
    }

    return $found_match;
  }
}
