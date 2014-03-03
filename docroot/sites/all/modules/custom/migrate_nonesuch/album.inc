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
