<?php

/**
 * @file
 * Provides a 'quote' link for Views.
 */

/**
 * Field handler to present a link to delete a node.
 */
class quote_handler_field_comment_link_quote extends views_handler_field_comment_link {
  function access() {
    // Check for permission to reply to comments.
    return user_access('post comments');
  }

  function render_link($data, $values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('quote');
    $nid =  $this->get_value($values, 'nid');
    $cid =  $this->get_value($values, 'cid');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "comment/reply/" . $nid . '/' . $cid . '?quote=1#comment-form';

    if (_quote_variable_get('highlighted_text')) {
      $this->options['alter']['link_class'] = 'quote-link';
      $this->options['alter']['link_attributes']['id'] = 'quote-link-comment-' . $cid;

      foreach ($this->entities as $entity) {
        if ($entity->cid == $cid) {
          $id = 'quote-link-comment-' . $entity->cid;
          $settings['quote'][$id]['author'] = $entity->name;
          $settings['quote'][$id]['body'] = $entity->comment_body[$entity->language][0]['value'];
          $settings['quote'][$id]['textarea'] = "#edit-comment-body-{$entity->language}-0-value";
        }
      }
      drupal_add_js($settings, 'setting');
    }

    return $text;
  }
}
