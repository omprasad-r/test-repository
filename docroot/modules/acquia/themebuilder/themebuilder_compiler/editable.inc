<?php
// $Id:$

/**
 * This interface tags asset implementations that represent editable properties.
 */
interface UserEditable {
  const CUSTOM = 0;
  const GENERATED = 1;
  public function getProperty($selector, $property_name);
  public function setProperty($selector, $property_name, $value, $generated = self::CUSTOM);
}

/**
 * This interface tags asset implementations that represent user selectable properties.
 */
interface UserSelectable {
  public function getSelection($selection_name);
  public function setSelection($selection_name, $selection_id);
}