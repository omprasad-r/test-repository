<?php
/**
 * @file
 * Mappings specific to theveronicas.
 */

class theveronicasRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6, 21521);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',
      '2' => '6',
      '3' => '6',
      '4' => '11',
    );

    $this->sourceDirectory =
      'http://img.wmgstg.com/wmgveronicas/files/imagecache/photo_full/';
  }

  protected function register() {
    $this->registerFile();
    $this->registerNode('news', 'blog');
  }
}

class theveronicasFileMigration extends AAFile6Migration {
  protected function query() {
    $query = parent::query();
    $query->innerJoin('content_type_news', 'ctn', 'f.fid=ctn.field_news_image_fid');
    return $query;
  }
}

class theveronicasnewsMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies = array('File');

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('News');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('status', NULL, FALSE)
         ->defaultValue(NODE_NOT_PUBLISHED);
    $this->addFieldMapping('uid', 'uid', FALSE)
         ->callbacks(array($this, 'mapAuthors'));

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      '1',
      'field_news_image', 'field_news_image:list', 'field_news_image:data',
      'field_news_video', 'field_news_video:value', 'field_news_video:provider',
        'field_news_video:data', 'field_news_video:version', 'field_news_video:duration',
    ));
  }

  protected function mapAuthors($source_uid) {
    $map = array(
      7536  => 21511, // sarahV
      25530 => 21446, // jessV
      25531 => 21441, // lisaV
      422   => 21416, // theveronicasadmin
      1     => 21416, // wmgtech => theveronicasadmin
      3391  => 21416, // wbradmin => theveronicasadmin
      12478 => 21416, // siteadmin=> theversonicasadmin
    );
    return $map[$source_uid];
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // If the former image field was populated, prepend the image
    if (isset($row->field_news_image)) {
      $new_fid = db_select('migrate_map_file', 'map')
                 ->fields('map', array('destid1'))
                 ->condition('sourceid1', $row->field_news_image)
                 ->execute()
                 ->fetchField();
      if ($new_fid) {
        $img_info = array(
          'type' => 'media',
          'view_mode' => 'media_large',
          'fid' => $new_fid,
          'attributes' => array(),
        );
        $media_tag = '[[' . drupal_json_encode($img_info) . ']]' . '<br />';
        if (is_array($row->body)) {
          $row->body = $media_tag . $row->body[0];
        }
        else {
          $row->body = $media_tag. $row->body;
        }
      }
    }
  }
}
