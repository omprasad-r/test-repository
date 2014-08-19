<?php


class CeeloMigration extends Migration {

  public function __construct($arguments) {
    parent::__construct($arguments);

    if (isset($this->arguments['default_user'])) {
      if (is_numeric($this->arguments['default_user'])) {
        $uid = $this->arguments['default_user'];
      } else {
        $user = user_load_by_name($this->arguments['default_user']);

        if (!empty($user)) {
          $uid = $user->uid;
        }
      }

      if (isset($uid)) {
        $this->addFieldMapping('uid')->defaultValue($uid);
      }
    }
  }
}