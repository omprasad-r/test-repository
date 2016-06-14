<?php

/**
 * @file
 * Instagram feeds importer
 */

/*
 * You must implement hook_migrate_api(), setting the API level to 2, for
 * your migration classes to be recognized by the Migrate module.
 */
function migrate_instagram_migrate_api()
{
    $api = array(
        'api' => 2,
        'groups' => array(
            'instagram' => array(
                'title' => t('Instagram')
            )
        ),
        'migrations' => array(
            'PhotoNode' => array(
                'class_name' => 'JSONMigration',
                'group_name' => 'instagram'
            )
        )
    );
    return $api;
}

class InstagramListJSON extends MigrateListJSON
{
    /**
     * Originally this class assumed that the results would be all that was in the JSON
     * object. This class has been modified to look inside the JSON object and find the
     * expected data within the ['data'] member.
     */
    protected function getIDsFromJSON(array $data)
    {
        $ids = array();
        foreach ($data['data'] as $item) {
            $ids[] = $item['created_time'];
        }
        return $ids;
    }
    
    public function computeCount()
    {
        $count = 0;
        if (empty($this->httpOptions)) {
            $json = file_get_contents($this->listUrl);
        } else {
            $response = drupal_http_request($this->listUrl, $this->httpOptions);
            $json     = $response->data;
        }
        if ($json) {
            $data = drupal_json_decode($json);
            if ($data) {
                $count = count($data['data']);
            }
        }
        return $count;
    }
    
}

class InstagramItemJSON extends MigrateItemJSON
{
    protected $data = array();
    /**
     * Two problems with the default getItem() - it reads and parses the JSON on
     * each call, which we don't want to do for multiple items in the file; and,
     * we need to retrieve a given item from the file according to its 'id' element.
     **/
    public function getItem($id)
    {
        // We cache the parsed JSON at $this->data.
        if (empty($this->data)) {
            $data = $this->loadJSONUrl($this->itemUrl);
            //$dataUpdated = $data['data'];
            if ($data) {
                // Let's index the array by the ID for easy retrieval.
                foreach ($data->data as $item) {
                    $this->data[$item->created_time] = $item;
                }
            } else {
                // Error-handling here....
            }
        }
        // Return the requested item
        if (isset($this->data[$id])) {
            return $this->data[$id];
            
        } else {
            return NULL;
        }
    }
    
}

class JSONMigration extends Migration
{
    /**
     * A constructor
     */
    public function __construct($arguments)
    {
        parent::__construct($arguments);
        
        $result = db_select('migrate_instagram', 'mi')->fields('mi', array(
            'url'
        ))->OrderBy('serial', 'DESC')->execute()->fetchAssoc();
        
        $url = $result["url"];
		
		$urlUpdated = isset($url) ? $url : "";
                 
        $http_options = array();
        
        $this->source = new MigrateSourceList(new InstagramListJSON($urlUpdated), new InstagramItemJSON($urlUpdated, $http_options), $this->fields());
        
        $this->destination = new MigrateDestinationNode('photo');
        
        //map for the migration        
        $this->map = new MigrateSQLMap($this->machineName, array(
            'created_time' => array(
                'type' => 'int',
                'not null' => true,
                'alias' => 'n'
            )
        ), MigrateDestinationNode::getKeySchema());
        
        $this->addFieldMapping('title', 'title');
        $this->addFieldMapping('body', 'link');
        $this->addFieldMapping('field_instagram_image', 'image');
        $this->addFieldMapping('field_instagram_image:source_dir');
        $this->addFieldMapping('field_instagram_image:file_class')->defaultValue('MigrateFileUri');
        $this->addFieldMapping('field_instagram_image:alt', 'image');
        $this->addFieldMapping('field_instagram_image:title', 'image');
    }
    
    /**
     * Return the fields (this is cleaner than passing in the array in the MigrateSourceList class above)
     * @return array
     */
    function fields()
    {
        return array(
            'created_time' => 'Unique id',
            'link' => 'Instagram link',
            'image' => 'Instagram image',
            'title' => 'Title of the content'
        );
    }
    
    /**    
     * Remap fields as needed
     * @param type $row
     */
    function prepareRow($row)
    {
        //Title trim to fit character restriction
        $title = substr(utf8_encode($row->caption->text), 0, 255);
        if ($title !== "") {
            $row->title = $title;
        } else {
            $row->title = "From instagram";
        }
        
        //$image      = $row->images->standard_resolution->url;
        $image      = substr($row->images->standard_resolution->url, 0, strpos($row->images->standard_resolution->url, "?"));
        $row->image = $image;
    }
}


/**
 * Implements hook_form().
 */

function migrate_instagram_form($form, &$form_state)
{
    
    $result = db_select('migrate_instagram', 'mi')->fields('mi', array(
        'url'
    ))->OrderBy('serial', 'DESC')->execute()->fetchAssoc();
    
    $url = $result["url"];
    
    $form['url'] = array(
        '#type' => 'textfield',
        '#title' => t('Import URL'),
        '#size' => 32,
        '#maxlength' => 255,
        '#default_value' => isset($url) ? $url : NULL,
        '#required' => TRUE,
        '#description' => t('Instagram import URL')
    );
    
    $form['submit'] = array(
        '#value' => 'Submit the Values',
        '#type' => 'submit'
    );
    return $form;
}

/**
 * Implements hook_form_submit().
 */
function migrate_instagram_form_submit($form, &$form_state)
{
    $url = $form_state['values']['url'];
    
    $result = db_select('migrate_instagram', 'mi')->fields('mi', array(
        'url'
    ))->OrderBy('serial', 'DESC')->execute()->fetchAssoc();
    
    if (!empty($result["url"])) {
        db_update('migrate_instagram')->fields(array(
            'url' => $url
        ))->execute();
        drupal_set_message(t('Import URL updated. Go to /admin/content/migrate to start importing contents'));
    } else {
        db_insert('migrate_instagram')->fields(array(
            'url'
        ))->values(array(
            'url' => $url
        ))->execute();
        drupal_set_message(t('Import Url set. Go to /admin/content/migrate to start importing contents'));
    }
}
