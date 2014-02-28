<?php

/**
 * @file
 * Provides PHPUnit tests for Acsf Site.
 */

class AcsfSiteTest extends PHPUnit_Framework_TestCase {

  public function setUp() {
    $files = array(
      __DIR__ . '/../classes/AcsfSite.inc',
      __DIR__ . '/../../acsf_variables/acsf_variables_mock.php',
      __DIR__ . '/../../acsf_log/classes/AcsfLog.inc',
      __DIR__ . '/../../acsf_events/classes/AcsfEvent.inc',
      __DIR__ . '/../../acsf_events/classes/AcsfEventDispatcher.inc',
      __DIR__ . '/../../acsf_events/classes/AcsfEventHandler.inc',
    );
    foreach ($files as $file) {
      require_once $file;
    }
  }

  /**
   * Tests that we can use the factory method to get a cached site.
   */
  public function testFactoryLoadCache() {
    $nid = 12345678;
    acsf_vset('acsf_site_nid', $nid);
    $site = AcsfSite::load($nid);
    $this->assertInstanceOf('AcsfSite', $site);

    $cache = AcsfSite::load();
    $this->assertSame($site, $cache); 
    $this->assertEquals($site->nid, $cache->nid);

    acsf_vdel('acsf_site_nid');
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

    $value = $site->nid;
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

  /**
   * Tests the save() method.
   */
  public function testSavedData() {
    $nid = 12345678;
    $string = 'test value';
    $site = new AcsfSite($nid);
    $site->custom = $string;
    $site->save();
    unset($site);

    $clone = new AcsfSite($nid);
    $this->assertEquals($clone->custom, $string);
  }

}

