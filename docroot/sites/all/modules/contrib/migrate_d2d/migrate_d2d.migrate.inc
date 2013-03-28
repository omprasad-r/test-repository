<?php
/**
 * @file
 * Base classes for all Drupal-to-Drupal migration classes.
 */

abstract class DrupalMigration extends DynamicMigration {
  /**
   * Connection key for the DatabaseConnection holding the source Drupal
   * installation.
   *
   * @var
   */
  protected $sourceConnection;

  /**
   * The major version of the Drupal install serving as the migration
   * source, e.g. '6'.
   *
   * @var int
   */
  protected $sourceVersion;

  /**
   * Options to be passed to source constructors.
   *
   * @var array
   */
  protected $sourceOptions = array();

  /**
   * An array of available source fields, beyond those in the base query.
   * Derived classes should populate this before calling the parent
   * constructor.
   *
   * @var array
   */
  protected $sourceFields = array();

  /**
   * Source type (bundle), if any.
   *
   * @var string
   */
  protected $sourceType = '';

  /**
   * While much of the version-specific work can be done in the leaf classes,
   * to share data and behavior among all classes for a given Drupal version
   * we use this helper object.
   *
   * @var DrupalVersion
   */
  protected $version;

  /**
   * Map format mappings from the source system to the destination. Automated
   * generation of these mappings is version-dependent (through Drupal 6 they
   * were identified by numeric IDs, Drupal 7 introduced machine names).
   *
   * @var array
   */
  protected $formatMappings = array();

  /**
   * Track whether or not we've already displayed an encryption warning
   *
   * @var bool
   */
  protected static $showEncryptionWarning = TRUE;

  /**
   * Required arguments:
   *
   * source_connection - Connection key for the DatabaseConnection holding the
   *  source Drupal installation.
   * source_version - Major version number (as an integer) of the source install.
   * machine_name - Machine name under which a particular migration is registered.
   * description - Description of the migration.
   *
   * Optional arguments:
   *
   * source_database - Array describing the source connection, to be defined in
   *  the constructor. If absent, the source connection is assumed to be established
   *  elsewhere (typically settings.php).
   * group - Migration group to add this migration to.
   * dependencies - Array of migrations that must be run before this one.
   * soft_dependencies - Array of migrations that should be listed before this one.
   * format_mappings - Array keyed by source format IDs or machine names, with
   *  the values being the corresponding D7 machine name. If unspecified,
   * source_options - Array to be passed as options to source constructors,
   *  overriding the defaults (map_joinable FALSE, cache_counts TRUE, cache_key
   *  derived from the machine name).
   * version_class - The name of a custom DrupalVersion class overriding the
   *  default derived from source_version.
   *
   * @param array $arguments
   */
  public function __construct($arguments) {
    $group = MigrateGroup::getInstance($arguments['group_name']);
    $group_arguments = $group->getArguments();
    $this->arguments = $group_arguments + $arguments;
    $this->sourceVersion = $this->arguments['source_version'];
    if (isset($this->arguments['version_class'])) {
      $version_class = $this->arguments['version_class'];
    }
    else {
      $version_class = 'DrupalVersion' . $this->sourceVersion;
    }
    $this->version = new $version_class($this->arguments);
    if (!isset($this->arguments['group'])) {
      $this->arguments['group'] = MigrateGroup::getInstance($this->arguments['group_name']);
    }
    elseif (!is_object($this->arguments['group'])) {
      $this->arguments['group'] = MigrateGroup::getInstance($this->arguments['group']);
    }
    parent::__construct($this->arguments['group']);
    $this->sourceConnection = $this->arguments['source_connection'];
    if (!empty($group_arguments['source_database'])) {
      $source_database = $group_arguments['source_database'];
      // Ensure value is encrypted.
      if (is_array($source_database)) {
        $serialized_source = serialize($source_database);
        $encrypted_source = $this->encrypt($serialized_source);
        $group_arguments['source_database'] = $encrypted_source;
        db_merge('migrate_group')
          ->key(array('name' => $group->getName()))
          ->fields(array(
              'arguments' => serialize($group_arguments),
          ))
          ->execute();
      }
      else {
        $decrypted_source = $this->decrypt($group_arguments['source_database']);
        $source_database = unserialize($decrypted_source);
      }
      Database::addConnectionInfo($this->sourceConnection, 'default', $source_database);
    }
    if (!empty($this->arguments['source_type'])) {
      $this->sourceType = $this->arguments['source_type'];
    }
    $this->description = $this->arguments['description'];
    if (!empty($this->arguments['dependencies'])) {
      $this->dependencies = $this->arguments['dependencies'];
    }
    if (!empty($this->arguments['soft_dependencies'])) {
      $this->softDependencies = $this->arguments['soft_dependencies'];
    }
    $this->sourceOptions = array('map_joinable' => FALSE, 'cache_counts' => TRUE,
                     'cache_key' => 'migrate_' . $this->machineName);
    if (!empty($this->arguments['source_options'])) {
      $this->sourceOptions = array_merge($this->sourceOptions,
                                         $this->arguments['source_options']);
    }

    if (!empty($this->arguments['format_mappings'])) {
      $this->formatMappings = $this->arguments['format_mappings'];
    }
    else {
      $this->formatMappings = $this->version->getDefaultFormatMappings();
    }
  }

  /**
   * Encrypt an incoming value. Detects for existence of the Drupal 'Encrypt'
   *  module or the mcrypt PHP extension.
   *
   * @param string $value
   * @return string The encrypted value.
   */
  protected function encrypt($value) {
    if (module_exists('encrypt')) {
      $value = encrypt($value);
    }
    else if (extension_loaded('mcrypt')) {
      // Mimic encrypt module to ensure compatibility
      $key = drupal_substr(variable_get('drupal_private_key', 'no_key'), 0, 32);
      $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
      $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
      $value = mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $value, MCRYPT_MODE_ECB, $iv);

      $encryption_array['text'] = $value;
      $encryption_array['method'] = 'mcrypt_rij_256'; // For forward compatibility with encrypt module
      $encryption_array['key_name'] = 'drupal_private_key'; // For forward compatibility with encrypt module
      $value = serialize($encryption_array);
    }
    else {
      if (self::$showEncryptionWarning) {
        MigrationBase::displayMessage(t('Your database credentials are not encrypted. Ensure the Encrypt Drupal module or mcrypt PHP extension is installed for this functionality.'), 'warning');
        self::$showEncryptionWarning = FALSE;
      }
    }
    return $value;
  }

  /**
   * Decrypt an incoming value.
   *
   * @param string $value
   * @return string The encrypted value
   */
  protected function decrypt($value) {
    if (module_exists('encrypt')) {
      $value = decrypt($value);
    }
    else if (extension_loaded('mcrypt')) {
      // Mimic encrypt module to ensure compatibility
      $encryption_array = unserialize($value);
      $method = $encryption_array['method']; // Not used right now
      $text = $encryption_array['text'];
      $key_name = $encryption_array['key_name']; // Not used right now

      $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
      $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
      $key = drupal_substr(variable_get('drupal_private_key', 'no_key'), 0, 32);
      $value = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, $text, MCRYPT_MODE_ECB, $iv);
    }
    else {
      if (self::$showEncryptionWarning) {
        MigrationBase::displayMessage(t('Your database credentials are not encrypted. Ensure the Encrypt Drupal module or mcrypt PHP extension is installed for this functionality.'), 'warning');
        self::$showEncryptionWarning = FALSE;
      }
    }
    return $value;
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }

  /**
   * @abstract
   * The base source query for this migration.
   *
   * @return QueryConditionInterface
   */
  abstract protected function query();

  /**
   * Field mapping callback: translate an incoming format ID (through D6) or
   * machine name (D7+) to a D7 format machine name.
   *
   * @param $format
   * @return string|array
   */
  protected function mapFormat($format) {
    if (!is_array($format)) {
      $format = array($format);
    }
    $result = array();
    foreach ($format as $format_value) {
      if (isset($format_value) && isset($this->formatMappings[$format_value])) {
        $result[] = $this->formatMappings[$format_value];
      }
      else {
        $result[] = NULL;
      }
    }
    // Only return an array if we have to
    if (count($result) > 1) {
      return $result;
    }
    else {
      return reset($result);
    }
  }

  /**
   * Check to see if a given module is enabled in the source installation.
   * @todo: move to DrupalVersion?
   *
   * @param $module
   *  Name of module to check.
   * @return boolean
   *  1 if it is enabled, 0 if not.
   */
  protected function moduleExists($module) {
    $exists = Database::getConnection('default', $this->sourceConnection)
              ->select('system', 's')
              ->fields('s', array('status'))
              ->condition('name', $module)
              ->condition('type', 'module')
              ->execute()
              ->fetchField();
    // Convert NULL to 0
    if (!$exists) {
      $exists = 0;
    }
    return $exists;
  }
}

/**
 * There should be an implementation of this abstract class, named
 * DrupalVersion{version #}, for each Drupal version supported as a source. It
 * will implement any functions needed by multiple version-specific classes
 * (e.g., nodes as well as users).
 */
abstract class DrupalVersion {
  /**
   * Arguments for the containing migration. Primarily of interest for
   * the source_connection.
   *
   * @var array
   */
  protected $arguments;

  /**
   * An array of information on CCK/core fields.
   *
   * @var array
   *  key - field -name
   *  value - array of information:
   *    label: User-visible description of the field
   *    type: type of the field
   *    columns: array of database columns for the field
   */
  protected $sourceFieldInfo = array();
  public function getSourceFieldInfo() {
    return $this->sourceFieldInfo;
  }

  /**
   * The entity type (node, user, etc.)
   *
   * @var string
   */
  protected $entityType;

  /**
   * The bundle (node type pre-D7) - article, blog, etc.
   *
   * @var string
   */
  protected $bundle;

  /**
   * Pass the migration class arguments through to the version class.
   *
   * @param $arguments
   */
  public function __construct($arguments) {
    $this->arguments = $arguments;
  }

  /**
   * @abstract
   * Returns an array keyed by the source system's format identifier (integer ID
   * or machine name), with the destination Drupal 7 machine name as the value.
   */
  abstract public function getDefaultFormatMappings();

  /**
   * @abstract
   * Given a source path (e.g., node/1234 or user/35), return the alias from
   * the source database.
   *
   * @param $pattern
   * @param $id
   */
  abstract public function getPath($source);

  /**
   * @abstract
   * Return the names and labels of all custom fields (CCK pre-D7, core fields
   * D7 and later) attached to the given entity type and bundle.
   *
   * @param $entity_type
   *  Type of entity ('node', 'user', etc.) for which to retrieve field info.
   * @param $bundle
   *  Bundle within the entity type (e.g., 'article', 'blog', etc.).
   *
   * @return array
   *  An array keyed by field name, with field labels as the values.
   */
  public function getSourceFields($entity_type, $bundle) {
    $this->populateSourceFieldInfo($entity_type, $bundle);
    $fields = array();
    foreach ($this->sourceFieldInfo as $field_name => $info) {
      $fields[$field_name] = $info['label'];
      $i = 0;
      if (isset($info['columns'])) {
        foreach ($info['columns'] as $display_name => $column_name) {
          // We skip the first column, which we've covered with the field name
          // itself.
          if ($i > 0) {
            $fields[$display_name] = t('!label subfield', array('!label' => $info['label']));
          }
          $i++;
        }
      }
    }
    return $fields;
  }

  /**
   * @abstract
   * Add CCK/core field values to the source row.
   *
   * @param $row
   * @param $entity_id
   *
   * @return array
   */
  abstract public function getSourceValues($row, $entity_id);
}

/**
 * Implements hook_migrate_api().
 */
function migrate_d2d_migrate_api() {
  $api = array('api' => 2);
  return $api;
}
