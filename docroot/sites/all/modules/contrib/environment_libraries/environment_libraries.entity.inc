<?php
/**
 * @file
 * Entity Info Definitions.
 */

/**
 * Implements hook_entity_info().
 */
function environment_libraries_entity_info() {

  $info['environment_libraries_library'] = array(
    'label' => t('Environment Library'),
    'base table' => 'environment_libraries_library',
    'controller class' => 'EntityEnvironmentLibrariesLibraryController',
    'uri callback' => 'environment_libraries_library_uri',
    'fieldable' => FALSE,
    'entity keys' => array(
      'id' => 'lid',
    ),
    'static cache' => TRUE,
  );

  if (module_exists('entitycache')) {
    $info['environment_libraries_library']['field cache'] = FALSE;
    $info['environment_libraries_library']['entity cache'] = TRUE;
  }

  return $info;
}

// # Entity Controller
/**
 * Custom controller for the environment_libraries_library entity.
 */
class EntityEnvironmentLibrariesLibraryController extends EntityAPIController {
  /**
   * Override Entity Save to serialize the config.
   *
   * @see EntityAPIController::save()
   */
  public function save($entity) {
    if (isset($entity->config)) {
      // $entity->config = serialize($entity->config);
    }
    if (isset($entity->dependencies) && is_array($entity->dependencies)) {
      $entity->dependencies = implode(',', $entity->dependencies);
    }
    EntityEnvironmentLibrariesLibraryController::resetCache();
    return parent::save($entity);
  }

  /**
   * Override Entity Load to unserialize the config.
   *
   * @see EntityAPIController::load()
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = array();
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $entity) {
      if (isset($entity->config)) {
        // $entity->config = unserialize($entity->config);
      }
    }
    return $entities;
  }

  /**
   * Overrides DrupalDefaultEntityController::cacheGet().
   */
  protected function cacheGet($ids, $conditions = array()) {
    $entities = array();
    // Load any available entities from the internal cache.
    // Override default function, due to core issue #1572466.
    if ($this->cacheComplete && $ids === FALSE && !$conditions) {
      return $this->entityCache;
    }
    return parent::cacheGet($ids, $conditions);
  }

}

/**
 * Implements the library admin uri callback.
 */
function environment_libraries_library_uri($lib) {
  $lid = $lib->lid;
  return array(
    'path' => "admin/config/system/environment_libraries/libraries/$lid/edit",
  );
}

/**
 * Implements the library block admin callback.
 *
 * @param $lib
 *   A library entity.
 *
 * @return array
 *   A menu callback path.
 */
function environment_libraries_block_uri($lib) {
  $lid = $lib->lid;
  return array(
    'path' => "admin/structure/block/manage/environment_libraries/environment_libraries_library__$lid/configure",
  );
}

// ## Entity CRUD
/**
 * Load a library entity.
 *
 * @param $lid
 *   A library entity id.
 * @return bool|mixed
 *
 */
function environment_libraries_library_load($lid) {
  if (!is_array($lid)) {
    $lid = array($lid);
  }
  $libraries = entity_load('environment_libraries_library', $lid, array(), FALSE);
  if (count($libraries) > 0) {
    return reset($libraries);
  }
  else {
    return FALSE;
  }
}

/**
 * Load a library entity by machine name.
 *
 * @param $machine_name
 *   The environment_library entity machine name.
 *
 * @return bool|mixed
 *   An environment_library entity.
 */
function environment_libraries_library_load_by_machine_name($machine_name) {
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'environment_libraries_library')
    ->propertyCondition('machine_name', $machine_name, '=')
    ->execute();

  if (isset($result['environment_libraries_library'])) {
    $lids = array_keys($result['environment_libraries_library']);
    $libraries = entity_load('environment_libraries_library', $lids, array(), FALSE);
    if (isset($libraries)) {
      return reset($libraries);
    }
  }
  return FALSE;
}

/**
 * @see https://api.drupal.org/api/drupal/includes%21common.inc/function/entity_load/7
 */
function environment_libraries_library_load_multiple($lids = FALSE, $conditions = array(), $reset = FALSE) {
  return entity_load('environment_libraries_library', $lids, $conditions, $reset);
}

/**
 * @see https://api.drupal.org/api/drupal/includes%21common.inc/function/entity_create/7
 */
function environment_libraries_library_create($values = array()) {
  $library = entity_create('environment_libraries_library', $values);
  return $library;
}

/**
 * @see http://www.drupalcontrib.org/api/drupal/contributions!entity!entity.module/function/entity_save/7
 */
function environment_libraries_library_save(&$library) {
  if (is_array($library)) {
    $library = (object) $library;
  }
  return entity_save('environment_libraries_library', $library);
}

/**
 * @see http://www.drupalcontrib.org/api/drupal/contributions!entity!entity.module/function/entity_delete/7
 */
function environment_libraries_library_delete(&$library) {
  entity_delete('environment_libraries_library', $library->lid);
}
