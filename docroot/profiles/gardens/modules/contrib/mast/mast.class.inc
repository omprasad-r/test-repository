<?php
/**
 * @file
 * Defines interfaces and classes used by the MASt module.
 */

/**
 * Defines a general MASt exception.
 */
class mastException extends Exception {}

/**
 * Defines MASt exception for erroneous parameters.
 */
class mastParamsException extends mastException {}

/**
 * Defines MASt exception for erroneous values.
 */
class mastValueException extends mastException {}

/**
 * Defines a factory to register implementing classes and structure exportables.
 */
class mastFactory {
  private static $classes;

  const FIELD_TYPE_FIELD_API = 1;

  const FIELD_TYPE_PROPERTY = 2;

  const FIELD_TYPE_EXTRA = 3;

  /**
   * Registers implementing classes via Drupal hooks.
   *
   * Modules wishing to register their own classes should implement
   * hook_mast_classes(), which should return an array with two required
   * elements: 'entities' and 'field_types'. A 'fields' element is optional and
   * is used for registering individual fields.
   *
   * The following is an example hook_mast_classes() implementation:
   *
   * @code
   * return array(
   *   'files' => array(
   *     drupal_get_path('module', 'yourmodule') . '/yourmodule.mast.inc',
   *   ),
   *   'entities' => array(
   *     'node' => 'mastEntityNode',
   *   ),
   *   'field_types' => array(
   *     'textfield' => 'mastFieldStandard',
   *     '*' => 'mastFieldStandard',
   *   ),
   *   'fields' => array(
   *     'field_blog_categories' => 'yourmoduleFieldSpecial',
   *   ),
   * );
   * @endcode
   */
  public static function registry() {
    if (!empty(self::$classes)) {
      return self::$classes;
    }

    $classes = array();
    $possible_classes = module_invoke_all('mast_classes');

    if (!empty($possible_classes['files'])) {
      foreach ($possible_classes['files'] as $path) {
        if (file_exists($path)) {
          include_once $path;
        }
      }
    }
    foreach (array('entities', 'field_types', 'fields') as $class_type) {
      $classes[$class_type] = array();
      if (!empty($possible_classes[$class_type])) {
        foreach ($possible_classes[$class_type] as $type => $suggested) {
          $classes[$class_type][$type] = is_array($suggested) ? array_pop($suggested) : $suggested;
        }
      }
    }

    drupal_alter('mast_classes', $classes);

    self::$classes = $classes;

    return self::$classes;
  }

  /**
   * Defines a factory to create entity structures based on the registry.
   */
  public static function createEntityStructure($entity_type, $bundle) {
    if (!isset(self::$classes)) {
      self::registry();
    }
    if (isset(self::$classes['entities'][$entity_type])) {
      $class = self::$classes['entities'][$entity_type];

      if (class_exists($class)) {
        return new $class($bundle);
      }
    }

    throw new mastException('MASt entity class "' . $class . '" not found.');
  }

  /**
   * Defines a factory to create field structures based on the registry.
   */
  public static function createFieldStructure($field_info, $instance_info) {
    $class = self::getMastFieldClass($field_info, $instance_info);

    if (class_exists($class)) {
      return new $class($field_info, $instance_info);
    }

    throw new mastException('MASt field class "' . $class . '" not found.');
  }

  /**
   * Determines the MASt class to be used for a field.
   */
  public static function getMastFieldClass($field_info, $instance_info) {
    if (!isset(self::$classes)) {
      self::registry();
    }
    $field_name = isset($instance_info['field_name']) ? $instance_info['field_name'] : '';
    $field_type  = isset($field_info['type']) ? $field_info['type'] : '';

    if (isset(self::$classes['fields'][$field_name])) {
      $class = self::$classes['fields'][$field_name];
    }
    elseif (isset(self::$classes['field_types'][$field_type])) {
      $class = self::$classes['field_types'][$field_type];
    }
    else {
      $class = 'mastFieldStandard';
    }

    return $class;
  }
}

/**
 * Defines an interface for an entity that MASt can expose.
 */
interface mastEntityStructureInterface {

  /**
   * Constructor.
   */
  public function __construct($bundle);

  /**
   * Adds an individual field to the entity structure.
   */
  public function addField($field_info, $instance_info);

  /**
   * Returns an individual field definition.
   *
   * @param String name.
   */
  public function getField($name);

  /**
   * Returns all field definitions.
   */
  public function getAllFields();

  /**
   * Returns whether or not the current user has permission to create an entity.
   *
   * @param string $entity_type.
   * @param string $bundle.
   */
  public function createPermitted($entity_type, $bundle);

  /**
   * Gets read permission.
   *
   * @param string $entity_type.
   * @param string $bundle.
   * @param string $entity.
   */
  public function readPermitted($entity_type, $bundle, $entity);

  /**
   * Gets update permission.
   *
   * @param string $entity_type.
   * @param string $bundle.
   * @param string $entity.
   */
  public function updatePermitted($entity_type, $bundle, $entity);

  /**
   * Gets delete permission.
   *
   * @param string $entity_type.
   *
   * @param string $bundle.
   *
   * @param string $entity.
   */
  public function deletePermitted($entity_type, $bundle, $entity);

  /**
   * Gets author override permission.
   *
   * @param string $entity_type.
   *
   * @param string $bundle.
   *
   * @param string $entity.
   */
  public function authorOverridePermitted($entity_type, $bundle, $entity = NULL);

  /**
   * Returns teaser text to represent the entity in a listing.
   *
   * @param $entity
   */
  public function getTeaser($entity);

  /**
   * Returns an image URL to represent the entity in a listing.
   *
   * @param entity
   */
  public function getImage($entity);

}

/**
 * Defines an entity that MASt can expose.
 */
abstract class mastEntityStructure implements mastEntityStructureInterface {

  protected $fields;
  protected $bundle;
  protected $required_fields = array();

  /**
   * Constructor.
   */
  public function __construct($bundle) {
    $this->bundle = $bundle;
    $this->addFields();
  }

  /**
   * Adds the fields belonging to the entity structure.
   *
   * Each implmenentation decides which fields belong in the entity structure.
   */
  protected abstract function addFields();

  /**
   * Adds an individual field to the entity structure.
   */
  public function addField($field_info, $instance_info) {
    $name = $instance_info['field_name'];
    if (mast_field_type_supported($field_info['type'])) {
      $this->fields[$name] = mastFactory::createFieldStructure($field_info, $instance_info);
      if ($instance_info['required']) {
        $this->required_fields[] = $name;
      }
    }
  }

  /**
   * Returns an individual field definition.
   *
   * @param String name.
   */
  public function getField($name) {
    return $this->fields[$name]->getMastField();
  }

  /**
   * Returns all field definitions.
   */
  public function getAllFields() {
    $fields = array();
    foreach ($this->fields as $name => $field) {
      if ($field->hasEditAccess()) {
        $fields[$name] = $field->getMastField();
      }
    }
    return $fields;
  }

}

/**
 * Defines a node form.
 */
class mastEntityStructureNode extends mastEntityStructure {
  protected function addFields() {
    $fields = mast_field_info_fields();
    $instances = mast_field_info_instances('node', $this->bundle);

    if (!$this->authorOverridePermitted('node', $this->bundle)) {
      unset($instances['author']);
    }

    foreach ($instances as $instance_info) {
      $field_info = $fields[$instance_info['field_name']];
      $this->addField($field_info, $instance_info);
    }
  }

  /**
   * Gets the create permission.
   */
  public function createPermitted($entity_type, $bundle) {
    return node_access('create', $bundle);
  }

  /**
   * Gets the author override permission.
   */
  public function authorOverridePermitted($entity_type, $bundle, $entity = NULL) {
    return user_access('administer nodes');
  }

  /**
   * Gets the read permission.
   */
  public function readPermitted($entity_type, $bundle, $entity) {
    return node_access('view', $entity);
  }

  /**
   * Gets the update permission.
   */
  public function updatePermitted($entity_type, $bundle, $entity) {
    return node_access('update', $entity);
  }

  /**
   * Gets the delete permission.
   */
  public function deletePermitted($entity_type, $bundle, $entity) {
    return node_access('delete', $entity);
  }

  /**
   * Gets the teaser of the node.
   */
  public function getTeaser($entity) {
    if (empty($entity->build)) {
      $entity->build = node_view($entity, 'teaser');
    }
    // Make sure not to render any of the node's links.
    $entity->build['#contextual_links']['#access'] = FALSE;
    $entity->build['links']['#access'] = FALSE;
    $elements = element_children($entity->build);
    foreach ($elements as $delta) {
      if (isset($entity->build[$delta]['#items']) && count($entity->build[$delta]['#items']) > 1) {
        $entity->build[$delta]['#theme'] = 'mast_field';
      }
    }
    // Avoid passing the node through node.tpl.php, so we don't get the title.
    unset($entity->build['#theme']);
    $rendered = drupal_render($entity->build);
    // Return just the first ~50 characters of the node's text; break on
    // word boundaries, and add an ellipsis.
    return truncate_utf8(strip_tags(html_entity_decode($rendered, ENT_QUOTES, 'UTF-8')), 50, TRUE, TRUE, 10);
  }

  /**
   * Gets the first image attached to the node.
   */
  public function getImage($entity) {
    if (empty($entity->build)) {
      $entity->build = node_view($entity, 'teaser');
    }
    // Look in all the fields in the teaser version of the node, in order.
    $children = element_children($entity->build, TRUE);
    foreach ($children as $child) {
      $field = $entity->build[$child];
      if ($file = $this->extractImage($field)) {
        return image_style_url('mobile_thumbnail', $file->uri);
      }
    }
    return '';
  }

  /**
   * Returns a file object for the first image in a field.
   *
   * @param $field
   *   A field array from a built entity.
   *
   * @return mixed
   *   The file object, or FALSE if none was found.
   */
  protected function extractImage($field) {
    if (empty($field['#field_type'])) {
      return FALSE;
    }

    // Look for images inside image, file, and Media module 1.x fields (Media
    // 2.x uses native file fields).
    switch ($field['#field_type']) {
      case 'image':
        // The first item inside an image field is guaranteed to be an image,
        // so delta 0 is fine to use.
        $fid = $field[0]['#item']['fid'];
        return file_load($fid);

      case 'media':
      case 'file':
        // Media and file fields may store non-image files, so we need to search
        // every item in the field.
        $children = element_children($field);

        foreach ($children as $delta) {
          $fid = $field[$delta]['#file']->fid;
          $file = file_load($fid);

          // Make sure this is an image file before returning it.
          if (strpos($file->filemime, 'image') === 0) {
            return $file;
          }
        }
        break;
    }

    return FALSE;
  }

  /**
   * Prepares field values for the node.
   *
   * @throws mastValueException
   */
  public function prepareValues($data, $check_required_fields = TRUE) {
    $fields = $this->getAllFields();
    $node_data = array('type' => $data['type']);

    if (isset($data['status'])) {
      $node_data['status'] = $data['status'];
    }

    // When updating an entity the required fields are not necessary, those
    // should be already present.
    if ($check_required_fields) {
      foreach ($this->required_fields as $required) {
        if (!isset($data[$required]) || (!count($data[$required]) || (is_string($data[$required]) && strlen(trim($data[$required])) == 0))) {
          // We are missing data for a required field. If it's the author field,
          // allow it to go through anyway; it's special-cased below.
          // @todo Allow any required field with a default value to be omitted
          // from the node data.
          if ($required !== 'author') {
            throw new mastValueException(t('One or more required fields are missing'));
          }
        }
      }
    }

    foreach ($data as $key => $value) {
      if (!isset($fields[$key])) {
        // Ignore anything that didn't come from our original schema.
        continue;
      }

      if (is_array($value)) {
        if (!isset($value['values']) && !isset($value['create'])) {
          throw new mastValueException(t('Incorrectly structured data: multiple values must be passed as the \'values\' property of a value array.'));
        }

        $new_values = array();
        if (isset($value['values'])) {
          $new_values = $value['values'];
        }
        if (isset($value['create'])) {
          $new_values = array_merge($new_values, $this->fields[$key]->createNewValues($value['create']));
        }
        $value = $new_values;
      }
      $value = $this->fields[$key]->prepareValue($value);
      $node_data[$key] = $value;
    }

    if (empty($node_data['author'])) {
      $node_data['author'] = $GLOBALS['user']->uid;
    }

    return $node_data;
  }
}

/**
 * Defines an interface for a field that MASt can expose.
 */
interface mastFieldInterface {

  public function __construct(array $field_info, array $instance_info);

  public function getMastField();

  public function options($start, $count);

  public function autoComplete($match, $count);

  public function prepareValue($value);

  public function createNewValues($values);

  public function hasEditAccess();

}

/**
 * Defines a standard MASt field implementation.
 *
 * Most fields can be handled in a standardized way.
 */
class mastFieldStandard implements mastFieldInterface {

  /**
   * Holds the name of the column to use when setting a value for this field.
   *
   * This is only needed for field types that store data in multiple columns, or
   * in one column called soemthing other than 'value' that Entity API does not
   * know about.
   */
  protected $value_column;

  /**
   * Holds the Field API field defintion.
   */
  protected $field_info;

  /**
   * Holds the Field API field instance definition.
   */
  protected $instance_info;

  /**
   * Holds the MASt field structure which is built by each implementing class
   * according to its own logic, using the field and instance definition.
   */
  protected $mast_field;

  /**
   * Constructor.
   *
   * @param Array $field_info
   *   Associative array holding the Field API defintion for this field.
   * @param Array $instance_info
   *   Associative array holding the instance info.
   */
  public function __construct(array $field_info, array $instance_info) {
    if (!isset($field_info['mast_field_type'])) {
      $field_info['mast_field_type'] = mastFactory::FIELD_TYPE_EXTRA;
    }
    $this->field_info = $field_info;
    $this->instance_info = $instance_info;
  }

  /**
   * Gets information about a field.
   *
   * @return array
   *   A list of attributes including:
   *     - type
   *     - label
   *     - cardinality
   *     - default
   *     - required
   */
  public function getMastField() {
    if (empty($this->mast_field)) {
      $this->buildMastField();
    }
    return $this->mast_field;
  }

  /**
   * Gets the resource path.
   */
  protected function resourcePath($name) {
    if (empty($this->endpoint)) {
      $this->endpoint = services_endpoint_load('mast');
    }
    if (isset($this->endpoint->resources[$name]['alias'])) {
      $name = $this->endpoint->resources[$name]['alias'];
    }
    return $this->endpoint->path . '/' . $name;
  }

  /**
   * Initializes the field with some base values.
   */
  protected function initializeMastField() {
    $this->mast_field = array(
      'type' => $this->field_info['type'],
      'label' => $this->instance_info['label'],
      'cardinality' => (string) $this->field_info['cardinality'],
      'required' => (bool) $this->instance_info['required'],
      'weight' => isset($this->instance_info['widget']['weight']) ? (string) $this->instance_info['widget']['weight'] : '0',
    );
  }

  /**
   * Builds the field.
   */
  protected function buildMastField() {
    $this->initializeMastField();

    if (isset($this->instance_info['default_value']) && isset($this->instance_info['default_value'][0]['value'])) {
      $this->mast_field['default'] = new stdClass;
      foreach ($this->instance_info['default_value'] as $data) {
        $this->mast_field['default']->key = $data['value'];
        $this->mast_field['default']->value = '';
      }
    }
  }

  /**
   * Validates auto-complete parameters.
   *
   * @throws mastParamsException
   */
  protected function autoCompleteValidateParams($match, $count) {
    $error = '';
    if (!is_numeric($count) || $count <= 0) {
      $error .= t('Invalid count param @count ', array('@count' => $count));
    }
    if ($error) {
      throw new mastParamsException($error);
    }
  }

  /**
   * Performs auto-complete functionality for this field.
   */
  public function autoComplete($match, $count) {
    $this->autoCompleteValidateParams($match, $count);

    // By default, there is nothing to match.
    return new stdClass;
  }

  /**
   * Validates the field's options parameters.
   *
   * @throws mastParamsException
   */
  protected function optionsValidateParams($start, $count) {
    $error = '';
    if (!is_numeric($start) || $start < 0) {
      $error .= t("Invalid start param @start ", array('@start' => $start));
    }
    if (!is_numeric($count) || $count <= 0) {
      $error .= t("Invalid count param @count ", array('@count' => $count));
    }
    if ($error) {
      throw new mastParamsException($error);
    }
  }

  public function options($start, $count) {
    $this->optionsValidateParams($start, $count);

    // By default, there are no options.
    return array(
      'last' => TRUE,
      'values' => new stdClass,
    );
  }

  /**
   * Prepares the field value.
   *
   * Ensures value is something that entity_metadata_wrapper will accept for
   * this field, and also calls the validation callback if one is set for the
   * field.
   *
   * @throws mastValueException
   */
  public function prepareValue($value) {
    if ($this->mast_field['cardinality'] == 1) {
      // If an array is passed as the value for a single-value field, find the
      // first scalar value and use that.
      while (is_array($value)) {
        // If there isn't a meaningful value set, set it to an empty string.
        if (empty($value)) {
          $value = '';
          break;
        }
        $value = $value[0];
      }
    }
    else {
      // Ensure the value is passed as an array.
      if (!is_array($value)) {
        $value = array($value);
      }
    }

    if (!$this->validate($value)) {
      throw new mastValueException('MASt value for field ' . $this->instance_info['field_name'] . ' does not validate.');
    }

    if (isset($this->value_column)) {
      $new_value = array();
      if (is_array($value)) {
        foreach ($value as $val) {
          $new_value[] = array($this->value_column => $val);
        }
      }
      else {
        $new_value[$this->value_column] = $value;
      }
      $value = $new_value;
    }
    return $value;
  }

  protected function validate($value) {
    // By default, don't do any validation.
    return TRUE;
  }

  /**
   * Creates new values for the field.
   *
   * Most field types won't have any concept of creating new things that can be
   * used as values for the field. Those that do e.g., term reference fields,
   * will override this to return the appropriate array of newly created values.
   */
  public function createNewValues($values) {
    return array();
  }

  public function hasEditAccess() {
    if ($this->field_info['mast_field_type'] == mastFactory::FIELD_TYPE_FIELD_API) {
      return field_access('edit', $this->field_info, $this->instance_info['entity_type']);
    }
    return TRUE;
  }

}

/**
 * Defines a reference field implementation.
 */
class mastUserReferenceField extends mastFieldStandard {
  // @todo Implement the user reference field.
}

/**
 * Defines a toggle field implementation.
 */
class mastToggleField extends mastFieldStandard {
  public function buildMastField() {
    $this->initializeMastField();
    $this->mast_field['options'] = array();
    $this->mast_field['options']['values'] = new stdClass;
    $this->mast_field['options']['values']->{'0'} = t('Off');
    $this->mast_field['options']['values']->{'1'} = t('On');

    $this->mast_field['default'] = new stdClass;
    if (isset($this->instance_info['default']) && isset($this->mast_field['options'][$this->instance_info['default']])) {
      $this->mast_field['default']->{$this->instance_info['default']} = $this->mast_field['options'][$this->instance_info['default']];
    }
    else {
      $this->mast_field['default']->{'1'} = t('On');
    }
    $this->mast_field['options']['icons'] = !empty($this->instance_info['icons']) ? $this->instance_info['icons'] : array();
  }
}

class mastFieldFormattedText extends mastFieldStandard {
  /**
   * Holds the name of the column to use when setting a value for this field.
   */
  protected $value_column = 'value';

  /**
   * Adds format to the formatted text field.
   */
  public function prepareValue($value) {
    $struct = parent::prepareValue($value);

    if ($this->mast_field['cardinality'] == 1) {
      $struct['format'] = filter_default_format();
    }
    else {
      foreach ($struct as $delta => $item) {
        $struct[$delta]['format'] = filter_default_format();
      }
    }

    return $struct;
  }
}

class mastDateField extends mastFieldStandard {
  public function buildMastField() {
    $this->initializeMastField();
    $this->mast_field['options'] = array();
    $this->mast_field['options']['date'] = FALSE;
    $this->mast_field['options']['time'] = FALSE;

    // Compress the granularity settings into just data and/or time.
    foreach ($this->field_info['settings']['granularity'] as $segment) {
      switch ($segment) {
        case 'year':
        case 'month':
        case 'day':
          $this->mast_field['options']['date'] = TRUE;
          break;
        case 'hour':
        case 'minute':
          $this->mast_field['options']['time'] = TRUE;
          break;
      }
    }
  }
}

/**
 * Generates list fields.
 *
 * Additional info is required for author fields.
 */
class mastListField extends mastFieldStandard {
  public function buildMastField() {
    $this->initializeMastField();
    $this->mast_field['options'] = array();
    $allowed_values = list_allowed_values($this->field_info, $this->instance_info, $this->instance_info['entity_type']);
    if (count($allowed_values) <= variable_get('mast_allowed_values_threshold', 10)) {
      $this->mast_field['options']['values'] = (object) $allowed_values;
    }
    else {
      $this->mast_field['options']['match'] = $this->resourcePath('autocomplete');
      $this->mast_field['options']['scroll'] = $this->resourcePath('options');
    }
    if (isset($this->instance_info['default_value'])) {
      $this->mast_field['default'] = new stdClass;
      foreach ($this->instance_info['default_value'] as $data) {
        $key = $data['value'];
        $value = $allowed_values[$key];
        $this->mast_field['default']->key = $key;
        $this->mast_field['default']->value = $value;
      }
    }
  }

  public function options($start, $count) {
    $this->optionsValidateParams($start, $count);
    $allowed_values = list_allowed_values($this->field_info, $this->instance_info, $this->instance_info['entity_type']);
    $options = array_slice($allowed_values, $start, $count);
    // Cast to object since we don't want to lose numeric keys.
    $result = array(
      'values' => (object) $options,
    );
    if (empty($options) || (count($allowed_values) - $start - $count <= 0)) {
      $result['last'] = TRUE;
    }
    return $result;
  }

  public function autoComplete($match, $count) {
    $this->autoCompleteValidateParams($match, $count);
    $allowed_values = list_allowed_values($this->field_info, $this->instance_info, $this->instance_info['entity_type']);
    $found = array();
    foreach ($allowed_values as $k => $v) {
      if (strpos($v, $match) === 0) {
        $found[$k] = $v;
      }
      if (count($found) >= $count) {
        break;
      }
    }
    // Cast to object since we don't want to lose numeric keys.
    return (object) $found;
  }

  /**
   * Removes the empty meta-value from the list field.
   */
  public function prepareValue($value) {
    $struct = parent::prepareValue($value);

    if ($this->mast_field['cardinality'] == 1) {
      if ($struct == '_none') {
        $struct = NULL;
      }
    }
    else {
      $index = array_search('_none', $struct, TRUE);
      if ($index !== FALSE) {
        unset($struct[$index]);
      }
    }

    return $struct;
  }

  protected function validate($value) {
    $allowed_values = list_allowed_values($this->field_info, $this->instance_info, $this->instance_info['entity_type']);
    if (!is_array($value)) {
      $value = array($value);
    }
    foreach ($value as $val) {
      if (!isset($allowed_values[$val]) && $val !== '_none') {
        return FALSE;
      }
    }
    return TRUE;
  }
}

/**
 * Boolean list fields.
 */
class mastListBooleanField extends mastListField {
  public function buildMastField() {
    parent::buildMastField();
    // The concept of a multi-value boolean field defies the foundations of logic
    // and undermines the integrity of the universe.
    $this->mast_field['cardinality'] = 1;
  }

  public function validate($value) {
    $acceptable_bool_values = array_merge($this->getAcceptableFalseValues(), $this->getAcceptableTrueValues());
    return in_array($value, $acceptable_bool_values, TRUE);
  }

  /**
   * Converts the value to either 1 or 0, as long as it passed validation as a
   * boolean value.
   */
  public function prepareValue($value) {
    $value = parent::prepareValue($value);
    // If we got this far, the value has passed validation, now just convert it
    // to a proper boolean value acceptable by the Field API boolean field.
    if (in_array($value, $this->getAcceptableTrueValues(), TRUE)) {
      return 1;
    }
    return 0;
  }

  /**
   * Returns an array of values that are acceptable as boolean FALSE.
   */
  protected function getAcceptableFalseValues() {
    return array(
      'false', 'False', 'FALSE', '0', false, 0, null
    );
  }

  /**
   * Returns an array of values that are acceptable as boolean TRUE.
   */
  protected function getAcceptableTrueValues() {
    return array(
      'true', 'True', 'TRUE', '1', true, 1
    );
  }
}

/**
 * Additional info is required for author fields.
 */
class mastAuthorField extends mastFieldStandard {
  public function buildMastField() {
    global $user;
    $this->initializeMastField();
    $this->mast_field['options'] = array();
    $this->mast_field['options']['match'] = $this->resourcePath('autocomplete');
    $this->mast_field['options']['scroll'] = $this->resourcePath('options');
    $this->mast_field['default'] = new StdClass;
    $this->mast_field['default']->key = $user->uid;
    $this->mast_field['default']->value = $user->name;
  }

  /**
   * Provide a listing of allowed authors for use in the author field.
   */
  public function options($start, $count) {
    $this->optionsValidateParams($start, $count);
    $users = array(
      'start' => $start,
      'values' => array()
    );

    $allowed_roles = variable_get('mast_settings_roles', array());
    $query = db_select('users', 'u');
    $query->fields('u', array('uid', 'name'));
    if (!empty($allowed_roles)) {
      // Restrict to only users with at least one of the allowed roles.
      $r_alias = $query->innerJoin('users_roles', 'r', '%alias.uid = u.uid');
      $query->condition("{$r_alias}.rid", $allowed_roles, 'IN');
    }
    // Exclude the anonymous user always.
    $query->condition('u.uid', '0', '!=');

    $user_list = $query->range($start, $count + 1)
      ->orderBy('u.name')
      ->execute()
      ->fetchAllKeyed();

    if (count($user_list) > $count) {
      // Discard the extra one.
      array_pop($user_list);
    }
    else {
      $users['last'] = TRUE;
    }
    if (!empty($user_list)) {
      $users['values'] = $user_list;
    }

    return $users;
  }

  public function autoComplete($match, $count) {
    $this->autoCompleteValidateParams($match, $count);
    $allowed_roles = variable_get('mast_settings_roles', array());
    $query = db_select('users', 'u');
    $query->fields('u', array('uid', 'name'));
    if (!empty($allowed_roles)) {
      // Restrict to only users with at least one of the allowed roles.
      $r_alias = $query->innerJoin('users_roles', 'r', '%alias.uid = u.uid');
      $query->condition("{$r_alias}.rid", $allowed_roles, 'IN');
    }

    $query->condition('name', db_like($match) . '%', 'LIKE');
    // Exclude the anonymous user always.
    $query->condition('u.uid', '0', '!=');

    $user_list = $query->range(0, $count)
      ->orderBy('u.name')
      ->execute()
      ->fetchAllKeyed();

    return $user_list;
  }

  /**
   * Validates the value passed in for the author field.
   */
  protected function validate($value) {
    if (!is_numeric($value)) {
      return FALSE;
    }
    $value = (int) $value;
    // We always allow the user to post on his/her own behalf, regardless of what's
    // in the mast_settings_roles variable.
    global $user;
    if ($value == $user->uid) {
      return TRUE;
    }
    if (!db_query('SELECT uid FROM {users} WHERE uid = :uid', array(':uid' => $value))->fetchField()) {
      return FALSE;
    }
    $allowed_roles = array_filter(variable_get('mast_settings_roles', array()));
    if (empty($allowed_roles)) {
      return TRUE;
    }

    $result = db_query('SELECT uid FROM {users_roles} WHERE uid = :uid AND rid IN (:rid)', array(':uid' => $value, ':rid' => array_keys($allowed_roles)))->fetchAll();
    return !empty($result);
  }
}

/**
 * Additional info is required for taxonomy fields.
 */
class mastTaxonomyTermReferenceField extends mastFieldStandard {
  public function buildMastField() {
    $this->initializeMastField();
    $this->mast_field['options'] = array();
    $this->mast_field['options']['match'] = $this->resourcePath('autocomplete');
    $this->mast_field['options']['scroll'] = $this->resourcePath('options');
    // The only flag we have for a "free tagging" field is the widget.
    if ($this->instance_info['widget']['type'] == 'taxonomy_autocomplete') {
      $this->mast_field['options']['create'] = TRUE;
    }
    if (isset($this->instance_info['default_value'])) {
      $this->mast_field['default'] = new stdClass;
      foreach ($this->instance_info['default_value'] as $data) {
        $this->mast_field['default']->key = $data['tid'];
        if (!isset($data['name'])) {
          $data['name'] = db_query("SELECT name from {taxonomy_term_data} WHERE tid = :tid", array(':tid' => $data['tid']))->fetchField();
        }
        $this->mast_field['default']->value = $data['name'];
      }
    }
  }

  public function options($start, $count) {
    $this->optionsValidateParams($start, $count);
    // Use an object here so empty JSON looks corect.
    $terms = array(
      'values' => new stdClass,
    );
    // Make sure the field exists and is a taxonomy field.
    if ($this->field_info['type'] !== 'taxonomy_term_reference') {
      return services_error(t('Taxonomy field @field_name not found.', array('@field_name' => $this->instance_info['field_name'])), 404);
    }
    // Part of the criteria for the query come from the field's own settings.
    $vids = array();
    $vocabularies = taxonomy_vocabulary_get_names();
    foreach ($this->field_info['settings']['allowed_values'] as $tree) {
      $vids[] = $vocabularies[$tree['vocabulary']]->vid;
    }

    $query = db_select('taxonomy_term_data', 't');
    $query->addTag('translatable');
    $query->addTag('term_access');

    // Select rows that match by term name.
    $term_matches = $query
      ->fields('t', array('tid', 'name'))
      ->condition('t.vid', $vids)
      // @todo Consider limiting or ordering based on popularity.
      ->orderBy('t.name')
      ->range($start, $count + 1)
      ->execute()
      ->fetchAllKeyed();

    if (count($term_matches) > $count) {
      // Discard the extra one.
      array_pop($term_matches);
    }
    else {
      $terms['last'] = TRUE;
    }
    if ($term_matches) {
      $terms['values'] = $term_matches;
    }

    return $terms;
  }

  public function autoComplete($match, $count) {
    $this->autoCompleteValidateParams($match, $count);
    $term_matches = array();
    // Make sure the field exists and is a taxonomy field.

    if ($this->field_info['type'] !== 'taxonomy_term_reference') {
      return services_error(t('Taxonomy field @field_name not found.', array('@field_name' => $this->instance_info['field_name'])), 404);
    }

    // The user enters a comma-separated list of tags. We only autocomplete the last tag.
    $tags_typed = drupal_explode_tags($match);
    $tag_last = drupal_strtolower(array_pop($tags_typed));

    $matches = array();
    if ($tag_last != '') {

      // Part of the criteria for the query come from the field's own settings.
      $vids = array();
      $vocabularies = taxonomy_vocabulary_get_names();
      foreach ($this->field_info['settings']['allowed_values'] as $tree) {
        $vids[] = $vocabularies[$tree['vocabulary']]->vid;
      }

      $query = db_select('taxonomy_term_data', 't');
      $query->addTag('translatable');
      $query->addTag('term_access');

      // Do not select already entered terms.
      if (!empty($tags_typed)) {
        $query->condition('t.name', $tags_typed, 'NOT IN');
      }
      // Select rows that match by term name.
      $term_matches = $query
        ->fields('t', array('tid', 'name'))
        ->condition('t.vid', $vids)
        ->condition('t.name', db_like($tag_last) . '%', 'LIKE')
        ->orderBy('t.name')
        ->range(0, $count)
        ->execute()
        ->fetchAllKeyed();

    }
    return $term_matches;
  }

  public function createNewValues($values) {
    $return_values = array();
    if ($this->mast_field['options']['create']) {
      $name = $this->field_info['settings']['allowed_values'][0]['vocabulary'];
      $vocabulary = taxonomy_vocabulary_machine_name_load($name);
      foreach ($values as $value) {
        $value = check_plain($value);
        if ($tid = db_query('SELECT tid FROM {taxonomy_term_data} WHERE name = :name AND vid = :vid', array(':name' => $value, ':vid' => $vocabulary->vid))->fetchField()) {
          $return_values[] = $tid;
        }
        else {
          $term = new stdClass();
          $term->name = $value;
          $term->vid = $vocabulary->vid;
          taxonomy_term_save($term);
          $return_values[] = $term->tid;
        }
      }
    }
    return $return_values;
  }

  /**
   * Handles taxonomy term field when it is emptied.
   */
  public function prepareValue($value) {
    $value = parent::prepareValue($value);
    if (empty($value)) {
      $value = NULL;
    }
    return $value;
  }
}

/**
 * Additional info is required for image fields.
 */
class mastImageField extends mastFieldStandard {
  /**
   * Holds the name of the column to use when setting a
   * value for this field.
   */
  protected $value_column = 'fid';

  public function buildMastField() {
    parent::buildMastField();
    $max_size = file_upload_max_size();
    // If we have a non-zero value set for the max filesize on this instance, use
    // that instead of the php max upload size.
    if (isset($this->instance_info['settings']['max_filesize']) && $this->instance_info['settings']['max_filesize']) {
      $max_size = $this->instance_info['settings']['max_filesize'];
    }
    $max_size = parse_size($max_size);
    if (empty($this->mast_field['options'])) {
      $this->mast_field['options'] = array();
    }
    $this->mast_field['options']['max_upload_size'] = $max_size;
    $allowed = array_map('trim', explode(' ', $this->instance_info['settings']['file_extensions']));
    $this->mast_field['options']['file_extensions'] = array_filter($allowed);
  }

  public function validate($value) {
    global $user;

    if (empty($value)) {
      return TRUE;
    }

    if (!is_array($value)) {
      $value = array($value);
    }

    foreach ($value as $fid) {
      if (!is_numeric($fid)) {
        return FALSE;
      }
      if (!$file = file_load($fid)) {
        return FALSE;
      }
      if ($file->uid != $user->uid) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Handles empty image fields.
   */
  public function prepareValue($value) {
    $struct = parent::prepareValue($value);

    if (empty($struct['fid'])) {
      $struct = NULL;
    }

    return $struct;
  }
}

/**
 * Class for handling media 1.x fields.
 */
class mastMediaField extends mastImageField {

}
