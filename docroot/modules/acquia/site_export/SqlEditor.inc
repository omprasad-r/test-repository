<?php

/**
 * The SqlEditor class exposes a relatively simple interface that can be used
 * to operate on mysqldump files.
 */
class SqlEditor {
  private $content;

  /**
   * Constructor for the SqlEditor class.
   *
   * @param <String> $content
   *   The database contents.  Note that this is passed by reference to avoid
   *   multiple copies of the potentially large data.
   */
  public function __construct(&$content) {
    $this->content = &$content;
  }

  /**
   * Drops the table from the database by removing the lines associated with
   * creating and populating the table.
   *
   * @param String $tableName
   *   The name of the table to drop.
   * @return SqlEditor
   *   This instance.
   */
  public function dropTable($tableName) {
    /*
     * Look for the pattern:
     * --
     * -- Table structure for table `tablename`
     * --
     *
     * <any number of non-empty lines>
     * <empty line>
     *
     * ... Replacing this pattern with an empty string will remove the sql
     * statements that construct the table.  Also remove the data that is
     * inserted into this table.
     */
    $dropTable = '/(?:^--\n-- Table structure for table [`]' . $tableName . '[`]\n--\n\n)(?:^[^\n]+\n)*(?:^$)*/m';
    $this->content = preg_replace($dropTable, '', $this->content);
    $this->dropTableData($tableName);
    return $this;
  }

  /**
   * Drops all data associated with the specified table.
   *
   * @param String $tableName
   *   The name of the table.
   * @return SqlEditor
   *   This instance.
   */
  public function dropTableData($tableName) {
    /*
     * Look for the pattern:
     * --
     * -- Dumping data for table `tablename`
     * --
     *
     * <any number of non-empty lines>
     * <empty line>
     *
     * ... Replacing this pattern with an empty string will remove the sql
     * statements that insert data into the table.
     */
    $dropTableData = '/(?:^--\n-- Dumping data for table [`]' . $tableName . '[`]\n--\n\n)(?:^[^\n]+\n)*(?:^$)*/m';
    $this->content = preg_replace($dropTableData, '', $this->content);
    return $this;
  }

  /**
   * Deletes the row in the specified table that contains the specified field value.
   *
   * @param String $tableName
   *   The name of the table.
   * @param String $fieldValue
   *   The field value used to identify the row to be deleted.
   * @param int $fieldOffset
   *   The index of the field (starting from 0) where the field value is expected
   *   to be found.
   * @return SqlEditor
   *   This instance.
   */
  public function dropTableRow($tableName, $fieldValue, $fieldOffset) {
    // Fix the fieldValue so it will work in a regular expression even if it has
    // regex special characters.
    $value = $this->sanitizeField($fieldValue);
    $dropRowData = '/^INSERT INTO [`]' .$tableName .'[`] VALUES [(]([^,]*,){' .$fieldOffset .'}\'' .$value .'\'.*$\n/m';
    $this->content = preg_replace($dropRowData, '', $this->content);
    return $this;
  }

  /**
   * Deletes the row in the specified table that contains the specified field value.
   *
   * @param String $tableName
   *   The name of the table.
   * @param String $fieldValue
   *   The field value used to identify the row to be deleted.
   * @param int $fieldOffset
   *   The index of the field (starting from 0) where the field value is expected
   *   to be found.
   * @return SqlEditor
   *   This instance.
   */
   public function dropTableRowIfFieldBeginsWith($tableName, $fieldValue, $fieldOffset) {
    // Fix the fieldValue so it will work in a regular expression even if it has
    // regex special characters.
    $value = $this->sanitizeField($fieldValue);
    $dropRowData = '/^INSERT INTO [`]' .$tableName .'[`] VALUES [(]([^,]*,){' .$fieldOffset .'}\'' .$value .'.*$\n/m';
    $this->content = preg_replace($dropRowData, '', $this->content);
    return $this;
  }

  /**
   * Converts the specified value into a string that can be safely inserted
   * into a regular expression.
   *
   * @param String $fieldValue
   *   The value to convert
   *
   * @return String
   *   The regex safe value.
   */
  public static function sanitizeField($fieldValue) {
    $value = '';
    foreach (str_split($fieldValue) as $c) {
      if ($c == '[' || $c == ']' || $c == '.' || $c == '/') {
        $value .= '\\' .$c;
      }
      else {
        $value .= $c;
      }
    }
    return $value;
  }

  /**
   * Affects the removal of the specified module by removing references to the
   * module in various tables.
   *
   * @param String $moduleName
   *   The name of the module to remove.
   * @return SqlEditor
   *   This instance.
   */
  public function removeModule($moduleName) {
    $path = drupal_get_path('module', $moduleName);
    $this->dropTableRow('system', $moduleName, 1)
      ->dropTableRowIfFieldBeginsWith('menu_router', $moduleName, 5)
      ->dropTableRowIfFieldBeginsWith('menu_links', str_replace('_', '-', $moduleName), 3)
      ->dropTableRowIfFieldBeginsWith('registry', $moduleName, 3);
    if (!empty($path)) {
      $this->dropTableRowIfFieldBeginsWith('locales_source', $path, 1);
    }
    $this->removeModuleMenus($moduleName);
    $this->removeModuleTables($moduleName);
    return $this;
  }

  /**
   * Removes menu entries associated with the specified module.
   *
   * @param String $moduleName
   *   The name of the module.
   *
   * @return SqlEditor
   *   This instance.
   */
  public function removeModuleMenus($moduleName) {
    $functionName = $moduleName .'_menu';
    if (function_exists($functionName)) {
      $menu = $functionName();
      foreach ($menu as $path => $value) {
        $this->dropTableRow('menu_links', $path, 3);
        $this->dropTableRow('menu_router', $path, 0);
      }
    }
    return $this;
  }

  /**
   * Removes all database tables associated with the specified module name.
   *
   * @param String $moduleName
   *   The module name associated with the tables to be removed.
   * @return SqlEditor
   *   This instance.
   */
  public function removeModuleTables($moduleName) {
    // The install files containing hook_schema are not normally loaded.
    module_load_include('install', $moduleName);
    $functionName = $moduleName . '_schema';
    if (function_exists($functionName)) {
      $schema = $functionName();
      foreach ($schema as $table_name => $table_data) {
        $this->dropTable($table_name);
      }
    }
    return $this;
  }

  /**
   * Adds an insert statement into the sql file.  The insert statement always
   * appears at the end of the file.
   *
   * @param String $tableName
   *   The name of the table to insert data into.
   * @param String $data
   *   A string representing the values for the fields in the table to add.
   * @return SqlEditor
   *   This instance.
   */
  public function insertTableData($tableName, $data) {
    $this->content .= "INSERT INTO `$tableName` VALUES ($data);\n";
    return $this;
  }

  /**
   * Adds an update statement into the sql file.  The update statement always
   * appears at the end of the file.  Beware that the original data will
   * still be in the SQL dump.
   *
   * @param {AssociativeArray} $statement
   *   A data structure containing the elements of an SQL update statement.
   * @return SqlEditor
   *   This instance.
   */
   public function updateTableData($statement) {
    $statement += array('data' => array(), 'func' => array(), 'where' => '');
    $sql = "UPDATE `{$statement['table']}` SET ";
    $conn = Database::getConnection();
    $set = array();
    // Data is for string update values that need to be quotes
    foreach ($statement['data'] as $field => $value) {
      $set[] = "`$field` = " . $conn->quote($value);
    }
    // Func is for things like function call replacements that can't be quoted.
    foreach ($statement['func'] as $field => $value) {
      $set[] = "`$field` = $value";
    }
    $sql .= implode(', ', $set);
    if ($statement['where']) {
      $sql .= " WHERE {$statement['where']}";
    }
    $this->content .= $sql . ";\n";
    return $this;
  }
}
