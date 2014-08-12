<?php


class CeeloMigration extends Migration {

  public function __construct($arguments) {
    parent::__construct($arguments);

    if (isset($this->arguments['default_user'])) {
      $this->addFieldMapping('uid')->defaultValue($this->arguments['default_user']);
    }
  }
}