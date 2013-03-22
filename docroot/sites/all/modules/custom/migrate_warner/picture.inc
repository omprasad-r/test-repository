<?php

/**
 * Base class implementing migration of user pictures from profile.xml.
 */
class PictureMigration extends XMLMigration {
  public function __construct() {
    ini_set('memory_limit', '512M');
    // Always call the parent constructor first for basic setup
    parent::__construct(MigrateGroup::getInstance('warner'));

    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mikeryan@cyrve.com', 'Cyrve'),
      new MigrateTeamMember('Pete Jelliffe', 'Pete.Jelliffe@wmg.com', 'Warner'),
      new MigrateTeamMember('Nik Gregory', 'Nik.Gregory@acquia.com', 'Acquia'),
    );

    // Fields available in profile.xml. Really, all we care about are the email
    // address and photo_url.
    $fields = array(
      'profile' => 'Email of user account',
      'photo_url' => 'URL of user picture',
    );

    // The source ID here is the one retrieved from each data item in the XML file, and
    // used to identify specific items
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'profile' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationFile::getKeySchema()
    );

    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');
    foreach (glob("$localpath/profile*.xml") as $items_url) {
      break;
    }
//    $item_xpath = '/file/profile[photo_url]';
    $item_xpath = '/file/profile';
    $item_ID_xpath = 'profile';
    $this->source = new MigrateSourceXML($items_url, $item_xpath, $item_ID_xpath,
      $fields, array('cache_counts' => TRUE));

    $this->destination = new MigrateDestinationFile(array('copy_file' => TRUE,
      'preserve_files' => TRUE));

    $this->addFieldMapping('uid')
         ->defaultValue(1);
    $this->addFieldMapping('uri', 'photo_url')
         ->xpath('photo_url');
    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array('filemime', 'contents', 'filename', 'path', 'timestamp'));
  }
  public function prepareRow($row) {
    if (empty($row->xml->photo_url)) {
      return FALSE;
    }
  }
  public function prepare($file, $row) {
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('uri', $file->uri)
           ->execute()
           ->fetchField();
    if ($fid) {
      $file->fid = $fid;
    }
    else {
      // First import of this file - derive a smaller version
      $style = image_style_load('profile_avatar_large');
      if (!image_style_create_derivative($style, $file->uri, $file->uri)) {
        $this->saveMessage(t('Failed to create smaller derivative of !uri',
                           array('!uri' => $file->uri)));
      }
    }
  }
}
