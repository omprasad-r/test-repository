<?php

/**
 * A utility to force wmode on object and embed tags in node body content.
 */
function gardens_enterprise_misc_force_wmode($wmode = 'opaque') {
  // Just return if the field_data_body table does not exist.
  if (!db_table_exists('field_data_body')) {
    return;
  }
  $results = db_select('field_data_body', 'fdb')
    ->fields('fdb')
    ->condition('fdb.body_value', '%object%', 'like')
    ->execute();
  // Loop through the results.
  foreach ($results as $key => $result) {
    $body = _gardens_enterprise_misc_force_wmode_clean_data($result->body_value);

    // Resave the node
    db_update('field_data_body')
      ->fields(array(
        'body_value' => $body
      ))
      ->condition('entity_id', $result->entity_id)
      ->execute();
  }
}

/**
 * Forces the wmode to the specified value for all object and embed tags in the
 * provides $data string.
 *
 * @param String $data
 *   Any string containing content such as HTML tags or bare text. If the string
 *   contains <object> or <embed> tags, these will be checked for the wmode
 *   setting and corrected if necessary.
 * @param String $wmode
 *   The desired wmode: opaque, transparent, window, etc.
 * @return String
 */
function _gardens_enterprise_misc_force_wmode_clean_data($data, $wmode = 'opaque') {
  // @TODO, Currently, I (jessebeach) cannot find a way to pass the $wmode
  // value to preg_replace_callback, so this value is hard-coded to transparent
  // in the two callback functions.

  // Clean <object>
  $data = preg_replace_callback('/<object.+?<\/object>/sm', '_gardens_enterprise_misc_force_wmode_clean_object', $data);

  // Clean <embed>
  $data = preg_replace_callback('/<embed.*?(:?\/|><\/embed)?>/sm', '_gardens_enterprise_misc_force_wmode_clean_embed', $data);

  return $data;
}

/**
 * Receives a preg_replace array with the text of a matched embed tag. The
 * embed tag is checked for the correct wmode attribute. If the attribute is not
 * present, it is added. If the attribute has the incorrect value, the value is
 * corrected. The embed tag string is returned either updated or not.
 *
 * @param Array $embed
 *   The preg_replace array. The zero index of the array is the complete string
 *   of the matched embed.
 *
 * @return String $embed
 *   The text of the matched embed tag.
 */
function _gardens_enterprise_misc_force_wmode_clean_embed($embed) {
  $wmode = 'transparent';
  $embed = $embed[0];
  $result = _gardens_enterprise_misc_force_wmode_check_attr($embed, $wmode);

  // wmode is missing.
  if (!$result['has_wmode']) {
    $attr = ' wmode="' . $wmode . '"';
    $embed = preg_replace('/(<embed)/sm', '$1' . $attr, $embed);
  }
  // wmode has the wrong value.
  if ($result['has_wmode'] && !$result['is_correct']) {
    $embed = preg_replace('/(.*?)(wmode\s*?\=\s*?[\"\'])(.*?)([\"\'].*)/sm', '$1$2' . $wmode . '$4', $embed);
  }

  return $embed;
}

/**
 * Receives a preg_replace array with the text of a matched object tag. The
 * object tag is checked for the correct wmode param. If the param is not
 * present, it is added. If the param has the incorrect value, the value is
 * corrected. The object tag string is returned either updated or not.
 *
 * @param Array $object
 *   The preg_replace array. The zero index of the array is the complete string
 *   of the matched object.
 *
 * @return String
 *   The text of the matched object tag.
 */
function _gardens_enterprise_misc_force_wmode_clean_object($object) {
  $wmode = 'transparent';
  $object = $object[0];
  $result = _gardens_enterprise_misc_force_wmode_check_params($object, $wmode);

  // wmode is missing.
  if (!$result['has_wmode']) {
    $param = '<param name="wmode" value="' . $wmode . '" />';
    $object = preg_replace('/(<object.*?>)/sm', '$1' . $param, $object);
  }
  // wmode has the wrong value.
  if ($result['has_wmode'] && !$result['is_correct']) {
    $object = preg_replace('/(.*?)(?<=param)(\s)+(name\s*?\=\s*?[\"\']wmode[\"\']\s+?value\s*?\=\s*?[\"\'])(.*?)([\"\'].*)/sm', '$1$2$3' . $wmode . '$5', $object);
  }

  return $object;
}

/**
 * Checks the text of an embed string for the wmode param and the correct
 * value of the wmode param. Returns an array of Booleans indicating if the
 * first or both of these conditions are met.
 *
 * @param String $object
 *   The object tag text.
 * @param String $wmode
 *   The wmode value to check for.
 * @return Array
 *   -has_wmode (Boolean): Whether the wmode param is present.
 *   -is_correct (Boolean): Whether the value of the wmode param matches the
 *   passed-in $wmode value.
 */
function _gardens_enterprise_misc_force_wmode_check_params($object, $wmode) {
  preg_match_all('/<param.*?(:?\/|><\/param)?>/sm', $object, $params);

  $result = array(
    'has_wmode' => false, // whether wmode param is present
    'is_correct' => false, // whether wmode param is configured to $wmode
  );

  foreach ($params[0] as $k => $param) {
    // Check if the wmode param exists.
    $name = preg_match('/name\s*?=\s*?[\"\']wmode[\"\']/', $param);
    if ($name > 0) {
      $result['has_wmode'] = true;
      // Check if the wmode param is set to $wmode as its value.
      $value = preg_match('/value\s*?=\s*?[\"\']' . $wmode . '[\"\']/', $param);
      if ($value > 0) {
        $result['is_correct'] = true;
      }
      break;
    }
  }

  return $result;
}

/**
 * Checks the text of an embed string for the wmode attribute and the correct
 * value of the wmode attribute. Returns an array of Booleans indicating if the
 * first or both of these conditions are met.
 *
 * @param String $embed
 *   The embed tag text.
 * @param String $wmode
 *   The wmode value to check for.
 * @return Array
 *   -has_wmode (Boolean): Whether the wmode attribute is present.
 *   -is_correct (Boolean): Whether the value of the wmode attribute matches the
 *   passed-in $wmode value.
 */
function _gardens_enterprise_misc_force_wmode_check_attr($embed, $wmode) {
  $result = array(
    'has_wmode' => false, // whether wmode param is present
    'is_correct' => false, // whether wmode param is configured to $wmode
  );

  $attr = preg_match('/\s+wmode\s*?=\s*?[\"\']\w+[\"\']/', $embed);
  // Check if the wmode attribute exists.
  if ($attr > 0) {
    $result['has_wmode'] = true;
    // Check if the wmode attribute is set to $wmode as its value.
    $check = preg_match('/\s+wmode\s*?=\s*?[\"\']' . $wmode . '[\"\']/', $embed);
    if ($check > 0) {
      $result['is_correct'] = true;
    }
  }

  return $result;
}
