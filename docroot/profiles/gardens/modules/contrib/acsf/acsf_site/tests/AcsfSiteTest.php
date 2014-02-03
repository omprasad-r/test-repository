<?php

/**
 * @file
 * Provides PHPUnit tests for Acsf Site.
 */

class AcsfSiteTest extends PHPUnit_Framework_TestCase {

  public function setUp() {
    $files = array(
      __DIR__ . '/../classes/AcsfSite.inc',
    );
    foreach ($files as $file) {
      require_once $file;
    }
  }

  /**
   * Tests that AcsfSite requires a node ID.
   *
   * @expectedException AcsfSiteMissingIdentifierException
   */
  public function testAcsfSiteConstructorRequirements() {
    $site = new AcsfSite();
  }

  /**
   * Tests the __get() method.
   */
  public function testAcsfSiteGet() {
    $site = new AcsfSite(12345678);
    $value = $site->__get('nid');
    $this->assertSame($value, 12345678);
  }

  /**
   * Tests the __set() method.
   */
  public function testAcsfSiteSet() {
    $site = new AcsfSite(12345678);

    $data = array(
      'true' => TRUE,
      'false' => FALSE,
      'string' => 'unit_test_string_value',
      'int' => mt_rand(0, 64),
      'float' => mt_rand() / mt_getrandmax(),
      'array' => array('foo', 'bar', 'baz', 'qux'),
    );
    $data['object'] = (object) $data;

    foreach ($data as $type => $value) {
      $site->__set($type, $value);
      $this->assertSame($site->__get($type), $value);
    }
  }
}

/**
 * Mocks acsf_vget_group for testing.
 */
function acsf_vget_group($group, $default = array()) {
  return $default;
}

