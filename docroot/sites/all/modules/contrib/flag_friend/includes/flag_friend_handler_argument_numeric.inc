<?php

/**
 * @file flag_friend_handler_argument_numeric.inc
 *
 * Contains the custom argument handler for the flag_friend table.
 */
class flag_friend_handler_argument_numeric extends views_handler_argument_numeric {
  function option_definition() {
    $options = parent::option_definition();
    $options['flag_friend_include_arg_user'] = array(
      'default' => 0,
      'translatable' => FALSE,
    );
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = $this->options;
    $form['flag_friend_include_arg_user'] = array(
      '#title' => t('Include argument user'),
      '#description' => t('Include results from the user whose friends will be shown.'),
      '#type' => 'checkbox',
      '#default_value' => $options['flag_friend_include_arg_user'],
    );
  }

  function query() {
    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array((int) $this->argument);
    }

    //Determine operator
    $operator = empty($this->options['not']) ? 'IN' : 'NOT IN';

    //Detect  if $value is array or not
    if (is_array($this->value)) {
      $value = $this->value;
    } else {
      $value = array($this->argument);
    }

    //Build array of all friends
    $all_friends = array();
    $all_friends_of = array();
    $all_friends += db_select('flag_friend', 'f')->fields('f', array('friend_uid'))->condition('uid', $value, $operator)->execute()->fetchCol();
    $all_friends_of += db_select('flag_friend', 'f')->fields('f', array('uid'))->condition('friend_uid', $value, $operator)->execute()->fetchCol();

    $all_friends = array_merge($all_friends, $all_friends_of);

    //Include argument if option is set.
    if ($this->options['flag_friend_include_arg_user']) {
      $all_friends[] = (int) $this->argument;
    }

    //If no friends, pass something or SQL borks
    if (count($all_friends) === 0){
      $all_friends = array(0);
    }

    //Determine field
    $field = 'users';
    $field .= $this->view->base_table != 'users'? '_' .$this->view->base_table: '';

    $this->query->add_where(0, $field . ".uid", $all_friends);
  }
}
