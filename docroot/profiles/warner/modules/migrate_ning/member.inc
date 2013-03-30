<?php
/**
 * @file
 * Support for migrating Ning members to Drupal user accounts.
 */

class NingMemberMigration extends NingMigration {
  /**
   * Machine name of the related picture migration, if any.
   *
   * @var string
   */
  protected $pictureMigration = '';

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  picture_migration (optional) - Machine name of the related picture
   *    migration, if any.
   */
  public function __construct($arguments) {
    $arguments['source_feature'] = 'members';
    parent::__construct($arguments);
    if (isset($arguments['picture_migration'])) {
      $this->pictureMigration = $arguments['picture_migration'];
      $this->softDependencies[] = $this->pictureMigration;
    }

    // Ideally, we would like to add any profileQuestions here for documentation
    // purposes, but it requires scanning the source, which is too time-consuming.
    // It is recommended that one does drush maz on the member migration to
    // identify profile fields that may be mapped.
    $this->sourceFields += array(
      'createdDate' => 'Account creation date',
      'fullName' => 'Member full name',
      'gender' => 'm or f',
      'country' => 'Two-letter country code',
      'zip' => 'Postal code',
      'birthdate' => 'Member birthdate',
      'email' => 'Email address',
      'level' => 'member/admin/owner',
      'state' => 'active/left/suspended',
      'contributorName' => 'Internal username',
    );

    $this->source = new MigrateSourceJSON($this->sourceFile, 'contributorName',
      $this->sourceFields, array('cache_counts' => TRUE, 'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'contributorName' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique page ID',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('name', 'contributorName');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('created', 'createdDate');
    $this->addFieldMapping('status', 'state')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('role_names', 'level')
         ->callbacks(array($this, 'handleRole'));
    if ($this->pictureMigration) {
      $this->addFieldMapping('picture', 'contributorName')
           ->sourceMigration($this->pictureMigration);
    }
    else {
      $this->addFieldMapping('picture');
      $this->addFieldMapping(NULL, 'contributorName');
    }

    $this->addUnmigratedDestinations(array(
      'access',
      'data',
      'init',
      'is_new',
      'language',
      'login',
      'pass',
      'roles',
      'signature',
      'signature_format',
      'theme',
      'timezone',
    ));

    if (module_exists('path')) {
      $this->addUnmigratedDestinations(array('path'));
      if (module_exists('pathauto')) {
        $this->addUnmigratedDestinations(array('pathauto'));
      }
    }
    if (module_exists('redirect')) {
      $this->addUnmigratedDestinations(array('migrate_redirects'));
    }
  }

  /**
   * Convert a Ning status keyword to Drupal status value.
   *
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    if ($value == 'active') {
      return 1;
    }
    else {
      return 0;
    }
  }

  /**
   * Convert a Ning member level to the Drupal user role.
   *
   * @param $value
   *
   * @return string
   */
  protected function handleRole($value) {
    switch ($value) {
      case 'admin':
      case 'owner':
        $value = 'administrator';
        break;
      case 'member':
      default:
        $value = 'authenticated user';
        break;
    }
    return $value;
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Flatten the profileQuestions object into top-level fields.
    if (isset($row->profileQuestions) && is_object($row->profileQuestions)) {
      foreach ($row->profileQuestions as $field_name => $field_value) {
        $field_name = $this->keyName($field_name);
        $row->$field_name = $field_value;
      }
      unset($row->profileQuestions);
    }
    if (isset($row->comments)) {
      unset($row->comments);
    }
  }

  /**
   * Convert a user-friendly field name to a lower-case alpha-numeric string
   * more suitable for specifying field mappings.
   * @todo: Cache the results.
   *
   * @param $field_name
   *
   * @return string
   */
  protected function keyName($field_name) {
    // Strip apostrophes.
    $key = str_replace("'", '', $field_name);
    // Turn into lower-case alpha-numerics with underscores
    $key = strtolower(preg_replace('|[^a-z0-9_]+|i', '_', $key));
    // Don't want any leading or trailing underscores.
    $key = trim($key, '_');
    return $key;
  }

  /**
   * Actions to take after a user account has been saved.
   *
   * @param $account
   * @param $row
   */
  public function complete($account, $row) {
    // If we've migrated a picture, update its ownership
    if (isset($this->pictureMigration)) {
      if ($account->picture) {
        db_update('file_managed')
          ->fields(array('uid' => $account->uid))
          ->condition('fid', $account->picture)
          ->execute();
      }
    }
  }
}

/**
 * Migrate comments on members to Drupal.
 */
class NingMemberCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'members';
    $arguments['parent_id_field'] = 'contributorName';

    parent::__construct($arguments);

  }

  /**
   * Implements Migration::prepare().
   *
   *
   *
   * @param $comment
   * @param $row
   */
  public function prepare($comment, $row) {
    // The "nid" we have here is the Drupal uid - get the comment_on_anything node
    $comment->nid = db_select('node', 'n')
                    ->fields('n', array('nid'))
                    ->condition('type', 'comment_on_anything')
                    ->condition('uid', $comment->nid)
                    ->execute()
                    ->fetchField();
  }
}
