<?php

/**
 * Field handler to provide User: picture URL field
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_picture_url extends views_handler_field {

  function element_type($none_supported = FALSE, $default_empty = FALSE, $inline = FALSE) {
    if ($inline) {
      return 'span';
    }

    return 'div';
  }

  function render($values) {
    $output = '';

    if ($picture_fid = $this->get_value($values)) {
      $picture = file_load($picture_fid);
      $picture_uri = $picture->uri;

      if ($picture_uri && file_valid_uri($picture->uri)) {
        $output = file_create_url($picture->uri);
      }
    }

    return $output;
  }
}
