<?php
/**
 * @file
 * Blog migration classes for googoodolls import.
 */

abstract class GooGooBlogMigration extends GooGooMigration {
  /**
   * List of source fields (keys) and their descriptions (values).
   *
   * @var array
   */
  protected $fields = array();

  /**
   * Name of the field containing the body ('text' or 'story').
   *
   * @var string
   */
  protected $bodyField = 'text';

  /**
   * For those blogs with pic1/pic2 fields, the directory holding the files.
   * @var
   */
  protected $blogDirectory;

  public function __construct($blog_table, $blog_category, $body_field = 'text') {
    parent::__construct();

    $this->bodyField = $body_field;

    $this->fields += array(
      'id' => 'Unique ID for the blog post',
      'date' => 'Date',
      $this->bodyField => 'Body'
    );

    // Note at least one empty row, identified by bogus date
    $query = Database::getConnection('default', 'googoo_site')
             ->select($blog_table, 'b')
             ->fields('b')
             ->condition('date', '0000-00-00', '<>');
    $this->source = new MigrateSourceSQL($query, $this->fields, NULL,
      array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('blog');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique blog post ID',
               ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('body', $this->bodyField);
    $this->addFieldMapping('body:format')
         ->defaultValue('6');    // Full HTML
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultAuthor);
    $this->addFieldMapping('created', 'date');
    $this->addFieldMapping('changed', 'date');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);
    $this->addFieldMapping('field_blog_category')
         ->defaultValue($blog_category);
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'comment',
      'daycount',
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    $cleaned_text = '';
    for ($i = 0; $i < strlen($row->{$this->bodyField}); $i++) {
      $char = $row->{$this->bodyField}[$i];
      switch (ord($char)) {
        case 145:
        case 146:
          $cleaned_text .= "'";
          break;
        case 151:
          $cleaned_text .= '&mdash;';
          break;
        case 169:
          $cleaned_text .= '&copy;';
          break;
        case 241:
          $cleaned_text .= '&ntilde;';
          break;
        default:
          $cleaned_text .= $char;
          break;
      }
    }
    $row->{$this->bodyField} = $cleaned_text;

    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $this->handleDate($row);

    // Unescape quotes
    if (isset($row->title)) {
      $row->title = str_replace("\'", "'", $row->title);
      $row->title = str_replace('\"', '"', $row->title);
    }
    $row->{$this->bodyField} = str_replace("\'", "'", $row->{$this->bodyField});
    $row->{$this->bodyField} = str_replace('\"', '"', $row->{$this->bodyField});
    if (isset($row->synopsis)) {
      $row->synopsis = str_replace("\'", "'", $row->synopsis);
      $row->synopsis = str_replace('\"', '"', $row->synopsis);
    }

    // Save the unmodified body for use by generateTitle before creating media tags.
    $row->originalBody = $row->{$this->bodyField};

    // Turn embedded images into media tags
    try {
      $row->{$this->bodyField} = preg_replace_callback('|<img [^>]*>|i',
        array($this, 'replaceCallback'), $row->{$this->bodyField});
    }
    catch (Exception $e) {
      $this->saveMessage($e->getMessage(), Migration::MESSAGE_INFORMATIONAL);
    }

    // If there are any attached pics, prepend them to the body
    foreach (array('pic1', 'pic2') as $pic_field) {
      if (isset($row->$pic_field)) {
        $arguments = array(
          'source_dir' => 'http://23.23.120.51/' . $this->blogDirectory,
          'destination_dir' => 'public://legacy',
          'destination_file' => $row->$pic_field,
          'file_replace' => MigrateFile::FILE_EXISTS_REUSE,
          'preserve_files' => TRUE,
        );
        $source = new MigrateFileUri($arguments);
        try {
          $file = $source->processFile($row->$pic_field, $this->defaultAuthor);
          $fid = $file->fid;
          $image_info = array(
            'type' => 'media',
            'view_mode' => 'media_original',
            'fid' => $fid,
            'attributes' => array(
              'alt' => '',
              'title' => '',
              'class' => 'media-image',
              'typeof' => 'foaf:Image',
              'wysiwyg' => 1,
            ),
          );
          $row->{$this->bodyField} = '[[' . drupal_json_encode($image_info) . ']]'
           . $row->{$this->bodyField};
        }
        catch (Exception $e) {
          $this->saveMessage($e->getMessage(), Migration::MESSAGE_INFORMATIONAL);
        }
      }
    }
  }

  /**
   * If a referenced image can be found in the files table, replace the <img> tag
   * with a media JSON reference (creating it if necessary).
   *
   * @param array $matches
   */
  protected function replaceCallback(array $matches) {
    $src_matches = array();
    // Default to the original <img> tag
    $result = $matches[0];
    // Extract the src parameter
    if (preg_match('|src=[\'"]([^\'"]+)[\'"]|i', $matches[0], $src_matches)) {
      $src = $src_matches[1];
      // Only try local references
      if (strncmp($src, 'http://', 7)) {
        $fid = db_select('file_managed', 'f')
               ->fields('f', array('fid'))
               ->condition('uri', '%' . $src, 'LIKE')
               ->execute()
               ->fetchField();
        if (!$fid) {
          $arguments = array(
            'source_dir' => 'http://23.23.120.51/site',
            'destination_dir' => 'public://legacy',
            'destination_file' => $src,
            'file_replace' => MigrateFile::FILE_EXISTS_REUSE,
            'preserve_files' => TRUE,
          );
          $source = new MigrateFileUri($arguments);
          $file = $source->processFile($src, $this->defaultAuthor);
          $fid = $file->fid;
        }
        if ($fid) {
          $image_info = array(
            'type' => 'media',
            'view_mode' => 'media_original',
            'fid' => $fid,
            'attributes' => array(
              'alt' => '',
              'title' => '',
              'class' => 'media-image',
              'typeof' => 'foaf:Image',
              'wysiwyg' => 1,
            ),
          );
          // Get the height and width parameters if present
          preg_match('|width=[\'"]([^\'"]+)[\'"]|i', $matches[0], $width);
          preg_match('|height=[\'"]([^\'"]+)[\'"]|i', $matches[0], $height);
          // image width
          if ($width) {
            $image_info['attributes']['width'] = $width[1];
          }
          // image height
          if ($height) {
            $image_info['attributes']['height'] = $height[1];
          }

          $result = '[[' . drupal_json_encode($image_info) . ']]';
        }
      }
    }
    return $result;
  }

  protected function generateTitle($body) {
    $stripped = trim(strip_tags($body));

    // Try the first "sentence"
    $title = trim(strtok($stripped, ".?!\n"));

    // If we find nothing, take the first word
    if (!$title) {
      $title = trim(strtok($title, ' '));
      // No match? Go back to the full stripped version
      if (!$title) {
        $title = $stripped;
      }
    }
    // Truncate to 100 chars, then remove the last (probably partial) word
    if (strlen($title) > 100) {
      $title = substr($title, 0, 100);
      $title = trim(substr($title, 0, strrpos($title, ' ')));
    }

    return $title;
  }
}

class GooGooBlogTourMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'title' => 'Title',
      'time' => 'Time',
    );
    parent::__construct('blog_something', 'Tour', 'story');
    $this->description = 'Import of tour blog posts from googoodolls.com';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'time')
         ->description('Appended to date');

  }
}

class GooGooBlogRoadieMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'title' => 'Title',
      'time' => 'Time',
    );
    parent::__construct('blog_roadie', 'Roadie', 'story');
    $this->description = 'Import of roadie blog posts from googoodolls.com';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'time')
         ->description('Appended to date');

  }
}

class GooGooBlogLLIMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'title' => 'Title',
      'pic1yes' => 'Y or N',
      'pic1' => 'Filename',
    );
    parent::__construct('blog', 'Let Love In Tour');
    $this->description = 'Import of LLI tour blog posts from googoodolls.com';

    $this->blogDirectory = 'member/blog/';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'pic1')
         ->description('Picture prepended in prepareRow');

    $this->addUnmigratedSources(array(
      'pic1yes',
    ));
  }
}

class GooGooBlogStudioMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'pic1yes' => 'Y or N',
      'pic1' => 'Filename',
      'pic2yes' => 'Y or N',
      'pic2' => 'Filename',
      'mp3yes' => 'Y or N',
      'mp3' => 'Filename',
      'vidyes' => 'Y or N',
      'vid' => 'Filename',
      'title' => 'Title generated from body',
    );

    parent::__construct('blog_studio', 'Let Love In Studio Diary');
    $this->description = 'Import of LLI studio diary blog posts from googoodolls.com';

    $this->blogDirectory = 'member/blog/studio/';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'pic1')
         ->description('Picture prepended in prepareRow');
    $this->addFieldMapping(NULL, 'pic2')
         ->description('Picture prepended in prepareRow');

    $this->addUnmigratedSources(array(
      'mp3',
      'mp3yes',
      'pic1yes',
      'pic2yes',
      'vid',
      'vidyes',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->title = $this->generateTitle($row->originalBody);
  }
}

class GooGooBlogGutterFlowerMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'pic1yes' => 'Y or N',
      'pic1' => 'Filename',
      'pic2yes' => 'Y or N',
      'pic2' => 'Filename',
      'mp3yes' => 'Y or N',
      'mp3' => 'Filename',
      'vidyes' => 'Y or N',
      'vid' => 'Filename',
      'title' => 'Title generated from body',
    );

    parent::__construct('blog_gutterflower', 'Gutterflower Tour');
    $this->description = 'Import of Gutterflower tour blog posts from googoodolls.com';

    $this->blogDirectory = 'member/blog/gutterflower/';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'pic1')
         ->description('Picture prepended in prepareRow');
    $this->addFieldMapping(NULL, 'pic2')
         ->description('Picture prepended in prepareRow');

    $this->addUnmigratedSources(array(
      'mp3',
      'mp3yes',
      'pic1yes',
      'pic2yes',
      'vid',
      'vidyes',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->title = $this->generateTitle($row->originalBody);
  }
}

class GooGooBlogDailyGooMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'pic1yes' => 'Y or N',
      'pic1' => 'Filename',
      'pic2yes' => 'Y or N',
      'pic2' => 'Filename',
      'mp3yes' => 'Y or N',
      'mp3' => 'Filename',
      'linkyes' => 'Y or N',
      'link' => 'URL',
      'title' => 'Title generated from body',
    );

    parent::__construct('blog_dailygoo', 'Daily Goo');
    $this->description = 'Import of Daily Goo blog posts from googoodolls.com';

    $this->blogDirectory = 'member/blog/dailygoo/';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'pic1')
         ->description('Picture prepended in prepareRow');
    $this->addFieldMapping(NULL, 'pic2')
         ->description('Picture prepended in prepareRow');

    $this->addUnmigratedSources(array(
      'mp3',
      'mp3yes',
      'pic1yes',
      'pic2yes',
      'link',
      'linkyes',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->title = $this->generateTitle($row->originalBody);
  }
}

class GooGooBlogNewsMigration extends GooGooBlogMigration {
  public function __construct() {
    $this->fields += array(
      'title' => 'Title',
      'time' => 'Time',
      'synopsis' => 'Usually the full content',
      'thumbnail' => 'Filename',
      'more' => 'Y or N',
    );
    parent::__construct('news', 'News', 'story');
    $this->description = 'Import of news posts from googoodolls.com';

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping(NULL, 'time')
         ->description('Appended to date');
    $this->addFieldMapping(NULL, 'synopsis')
         ->description('Prepended to story');

    $this->addUnmigratedSources(array(
      'more',
      'thumbnail',  // Not used on the live site
    ));
  }

  /**
   * Implements Migration:;prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Construct the body text from the synopsis and story fields.
    $row->story = $row->synopsis . '<br />' . $row->story;
  }
}
