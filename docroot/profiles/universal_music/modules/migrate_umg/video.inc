<?php
/**
 * @file
 * Video migration classes (from Youtube feed) for UMG.
 */

/**
 * Class for video migrations from Youtube feeds.
 */
class UMGVideoMigration extends XMLMigration {
  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct(MigrateGroup::getInstance($arguments['group_name']));

    $this->description = t('Migrate videos from Youtube to video nodes');

    $fields = array(
      // Fields directly retrieved from the XML
      'id' => 'Unique URL to ATOM data for this video',
      'published' => 'Published timestamp',
      'updated' => 'Updated timestamp',
      'app_control' => 'Information on restrictions',
      'category' => 'Categories for this video',
      'title' => 'Title of the video',
      'content' => 'Description of the video',
      'description' => 'Description from the media_group element',
      'duration' => 'Length of the video in seconds',
      'rating_average' => 'Average rating value, from 1-5, as a float',
      'rating_count' => 'Number of ratings going into the average',
      'view_count' => 'Number of video views',
      // Constructed fields
      'video_url' => 'The user-facing video URL',
      'author_name' => 'Youtube username',
      'author_url' => 'Link to Youtube user account',
    );

    // Build the Youtube API endpoint, and see if there are multiple pages.
    $urls[] = 'http://gdata.youtube.com/feeds/api/users/' .
      $arguments['youtube_feed_username'] . '/uploads?v=2&max-results=50';
    $xml = simplexml_load_file($urls[0]);
    $xml->registerXPathNamespace('feed', 'http://www.w3.org/2005/Atom');
    while ($xml && $link_xml = @$xml->xpath("feed:link[@rel='next']/@href")) {
      $link = $link_xml[0]['href'];
      if (is_array($link)) {
        $link = reset($link);
      }
      $link = (string)$link;
      $urls[] = $link;
      $xml = simplexml_load_file($link);
      $xml->registerXPathNamespace('feed', 'http://www.w3.org/2005/Atom');
    }
    $item_xpath = '/feed/entry';
    $item_ID_xpath = 'id';
    $this->source = new MigrateSourceXML($urls, $item_xpath, $item_ID_xpath,
      $fields);

    $this->destination = new MigrateDestinationNode('video');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'Video ID',
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('uid')
         ->defaultValue($arguments['default_author_id']);
    $this->addFieldMapping('created', 'published');
    $this->addFieldMapping('changed', 'updated');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_video', 'video_url')
         ->xpath("media:group/media:content[@yt:format='5']/@url");
    $this->addFieldMapping('field_video:file_class')
         ->defaultValue('MigrateExtrasFileYoutube');
    $this->addFieldMapping('field_video:title', 'title');
    $this->addFieldMapping('field_control', 'app_control')
         ->xpath('app:control/yt:state');
    $this->addFieldMapping('body', 'content');
    $this->addFieldMapping('field_duration', 'duration')
         ->xpath('media:group/yt:duration/@seconds');
    $this->addFieldMapping('field_video_author', 'author_url');
    $this->addFieldMapping(NULL, 'author_name')
         ->description('Used to construct link field');
    $this->addFieldMapping('field_description', 'description')
         ->xpath('media:group/media:description');
    $this->addFieldMapping('field_video_category', 'category')
         ->xpath('media:group/media:category');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(1);
    $this->addFieldMapping('field_video_rating', 'rating_average')
         ->xpath('gd:rating/@average');
    $this->addFieldMapping('field_rating_quantity', 'rating_count')
         ->xpath('gd:rating/@numRaters');
    $this->addFieldMapping('field_video_view_count', 'view_count')
         ->xpath('yt:statistics/@viewCount');

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'comment',
        'field_control:language',
        'field_description:language',
        'field_video:language', 'field_video:data',
        'field_video_category:source_type', 'field_video_category:ignore_case',
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
  }

  /**
   * Implements Migration::generateMachineName().
   *
   * @return string
   */
  protected function generateMachineName() {
    return $this->arguments['machine_name'];
  }

  /**
   * Implements Migration::prepareKey().
   *
   * @param array $key
   * @param object $row
   *
   * @return array
   */
  public function prepareKey($key, $row) {
    // We need to get the key populated right up front here, for --update to work.
    $row->id = (string)$row->xml->id;
    $key['id'] = $row->id;
    return $key;
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) == FALSE) {
      return FALSE;
    }

    // Some xpaths are not working for some mysterious reason, populate their
    // fields here.
    $row->title = (string)$row->xml->title;
    $row->published = (string)$row->xml->published;
    $row->updated = (string)$row->xml->updated;
    $row->content = (string)$row->xml->content;
    $row->author_name = (string)$row->xml->author->name;
    $row->author_url = 'http://www.youtube.com/user/' . $row->author_name;
  }

  /**
   * Implements Migration::prepare().
   *
   * @param $node
   * @param $row
   */
  public function prepare($node, $row) {
    // Link migration handler is missing in Gardens, do it by hand.
    $node->field_video_author = array(LANGUAGE_NONE => array(
      array(
        'url' => $row->author_url,
        'title' => $row->author_name,
      ),
    ));
  }
}
