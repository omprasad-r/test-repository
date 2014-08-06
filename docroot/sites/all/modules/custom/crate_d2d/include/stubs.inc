<?php
/**
 * @file
 * Holds the stub classes for all of the possible d2d migrate classes.
 */

class CrateD2DStubResolver {
  protected static $replacedClasses = array(
    'DrupalComment7Migration' => 'CrateD2DComment7Migration',
    'DrupalComment6Migration' => 'CrateD2DComment6Migration',
    'DrupalComment5Migration' => 'CrateD2DComment5Migration',
    'DrupalNode7Migration'    => 'CrateD2DNode7Migration',
    'DrupalNode6Migration'    => 'CrateD2DNode6Migration',
    'DrupalNode5Migration'    => 'CrateD2DNode5Migration',
    'DrupalTerm7Migration'    => 'CrateD2DTerm7Migration',
    'DrupalTerm6Migration'    => 'CrateD2DTerm6Migration',
    'DrupalTerm5Migration'    => 'CrateD2DTerm5Migration',
    'DrupalFile7Migration'    => 'CrateD2DFile7Migration',
    'DrupalFile6Migration'    => 'CrateD2DFile6Migration',
    'DrupalFile5Migration'    => 'CrateD2DFile5Migration',
    'DrupalUser7Migration'    => 'CrateD2DUser7Migration',
    'DrupalUser6Migration'    => 'CrateD2DUser6Migration',
    'DrupalUser5Migration'    => 'CrateD2DUser5Migration',
    'DrupalPicture7Migration'    => 'CrateD2DPicture7Migration',
    'DrupalPicture6Migration'    => 'CrateD2DPicture6Migration',
    'DrupalPicture5Migration'    => 'CrateD2DPicture5Migration',
    'DrupalRole7Migration'    => 'CrateD2DRole7Migration',
    'DrupalRole6Migration'    => 'CrateD2DRole6Migration',
    'DrupalRole5Migration'    => 'CrateD2DRole5Migration',
  );

  /**
   * Returns any classes that should be replaced by the drush command.
   *
   * @param string $class_name
   *   Name of the class to be replaced.
   * @return string
   *   Either the original class name or the replacement one, as appropriate.
   */
  public static function getReplacedClass($class_name) {
    if (isset(self::$replacedClasses[$class_name])) {
      return self::$replacedClasses[$class_name];
    }

    return $class_name;
  }
}

class CrateD2DComment7Migration extends DrupalComment7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DComment6Migration extends DrupalComment6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DComment5Migration extends DrupalComment5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DNode7Migration extends DrupalNode7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DNode6Migration extends DrupalNode6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DNode5Migration extends DrupalNode5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DTerm7Migration extends DrupalTerm7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DTerm6Migration extends DrupalTerm6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DTerm5Migration extends DrupalTerm5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DFile7Migration extends DrupalFile7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DFile6Migration extends DrupalFile6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DFile5Migration extends DrupalFile5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DUser7Migration extends DrupalUser7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DUser6Migration extends DrupalUser6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DUser5Migration extends DrupalUser5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DPicture7Migration extends DrupalPicture7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DPicture6Migration extends DrupalPicture6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DPicture5Migration extends DrupalPicture5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DRole7Migration extends DrupalRole7Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DRole6Migration extends DrupalRole6Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}

class CrateD2DRole5Migration extends DrupalRole5Migration {
  /**
   * Overloads the standard prepareRow().
   *
   * @param StdClass $row
   *   The migration source row.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $handler = CrateD2DMigrateHandler::getInstance(Migration::getInstance($this->machineName));

    $handler->runFieldModifiers($row);
  }
}
