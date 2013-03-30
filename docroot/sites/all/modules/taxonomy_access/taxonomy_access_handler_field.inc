<?php

class taxonomy_access_handler_field_term_grants extends views_handler_field_prerender_list {
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->additional_fields['taxonomy_access_tid'] = array(
      'table' => 'taxonomy_term_data',
      'field' => 'tid',
    );
  }

  function query() {
    $this->add_additional_fields();
  }

  function pre_render(&$values) {
    $this->field_alias = $this->aliases['taxonomy_access_tid'];
    $tids = array();
    foreach ($values as $result) {
      if (!empty($result->{$this->aliases['taxonomy_access_tid']})) {
        $tids[] = $result->{$this->aliases['taxonomy_access_tid']};
      }
    }

    if ($tids) {
      // TODO: Rewrite query to pull in default grants as well.
      $result = db_query('SELECT t.tid, t.rid, r.name, grant_view FROM {taxonomy_access_term} t JOIN {role} r ON r.rid = t.rid WHERE t.tid IN (' . implode(',', $tids) . ')');

      foreach ($result as $grant) {
        if ($grant->grant_view & TAXONOMY_ACCESS_NODE_ALLOW) {
          $this->items[$grant->tid][$grant->rid]['rid'] = $grant->rid;
          $this->items[$grant->tid][$grant->rid]['name'] = $grant->name;
        }
      }
    }
  }

  function render_item($count, $item) {
    return $item['name'];
  }

  function document_self_tokens(&$tokens) {
    $tokens['[' . $this->options['id'] . '-rid' . ']'] = t('The role ID.');
    $tokens['[' . $this->options['id'] . '-name' . ']'] = t('The role name.');
  }

  function add_self_tokens(&$tokens, $item) {
    foreach(array('rid', 'name') as $token) {
      $tokens['[' . $this->options['id'] . '-' . $token . ']'] = isset($item[$token]) ? $item[$token] : '';
    }
  }
}
