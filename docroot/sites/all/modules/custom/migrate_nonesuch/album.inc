<?php

/**
 * @file
 * Special handling to turn UPCs and prices into collection fields.
 */
class MigrateNonesuchAlbum extends DrupalNode5Migration {
  /**
   * Implements Migration::complete().
   *
   * @param $node
   * @param $row
   */
  public function complete($node, $row) {
    // The various UPC types from the legacy site, ordered by field weights
    // so the collection items maintain that order.
    $types = array(
      'physical' => 'SINGLE FORMAT (OLD)',
      'cdmp3' => 'CD+MP3',
      'download' => 'MP3',
      'lpcdmp3' => 'LP+CD+MP3',
      'dvd' => 'DVD',
      'cddvd' => 'CD+DVD',
      'cd' => 'CD',
      'lp' => 'LP',
      'flac' => 'FLAC (Lossless)',
    );

    foreach ($types as $type => $label) {
      $upc_field = 'field_album_upc_' . $type;
      $upc = $row->$upc_field;

      $price_field = 'field_album_price_' . $type;
      $price = $row->$price_field;
      if (!$label) {
        $label_field = 'field_album_label_' . $type;
        $label = $row->$label_field;
      }
      if ($upc || $price) {
        $this->addCollectionItem($node, $upc, $price, $label);
      }
    }

    // Wildcard is a special case - potentially multiple, with an explicit
    // label.
    foreach ($row->field_album_upc_wildcard as $delta => $upc) {
      $price = $row->field_album_price_wildcard[$delta];
      $label = $row->field_album_label_wildcard[$delta];
      $this->addCollectionItem($node, $upc, $price, $label);
    }
  }

  /**
   * Add one upc/price collection item to a node.
   *
   * @param $node
   * @param $upc
   * @param $price
   * @param $label
   */
  protected function addCollectionItem($node, $upc, $price, $label) {
    if ($upc || $price) {
      $entity_values = array(
        'field_name' => 'field_upc_price',
        'field_upc' => array(
          LANGUAGE_NONE => array(array('value' => $upc))),
        'field_price' => array(
          LANGUAGE_NONE => array(array('value' => $price / 100))),
        'field_label' => array(
          LANGUAGE_NONE => array(array('value' => $label)),
        )
      );
      $entity = entity_create('field_collection_item', $entity_values);
      $entity->setHostEntity('node', $node);
      $entity->save();
    }
  }
}

class MigrateNode7NonesuchAlbum extends DrupalNode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
  }

  /**
   * Implements Migration::complete().
   */
  public function complete($node, $row) {
    $query = Database::getConnection('default', $this->sourceConnection)
      ->select('field_data_field_upc_price', 'up');
    $query->leftJoin('field_data_field_label', 'la', 'la.entity_id = up.field_upc_price_value');
    $query->fields('la', array('field_label_value'));
    $query->leftJoin('field_data_field_upc', 'ua', 'ua.entity_id = up.field_upc_price_value');
    $query->fields('ua', array('field_upc_value'));
    $query->leftJoin('field_data_field_price', 'p', 'p.entity_id = up.field_upc_price_value');
    $query->fields('p', array('field_price_value'));
    $query->condition("up.entity_id", $row->nid);
    $results = $query->execute();
    foreach ($results as $result) {
      $this->addCollectionItem($node, $result->field_upc_value, $result->field_price_value, $result->field_label_value);
    }
  }

  /**
   * Add one upc price field collection item to a node.
   */
  protected function addCollectionItem($node, $upc, $price, $label) {
    $fc_item = entity_create('field_collection_item', array('field_name' => 'field_upc_price'));
    $fc_item->setHostEntity('node', $node);
    $collection = entity_metadata_wrapper('field_collection_item', $fc_item);
    $collection->field_upc_album->set($upc);
    $collection->field_price_album->set($price);
    $collection->field_label->set($label);
    $collection->save();
    node_save($node);
  }
}