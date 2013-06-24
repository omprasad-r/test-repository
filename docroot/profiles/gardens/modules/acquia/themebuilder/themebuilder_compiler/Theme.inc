<?php

include_once('assets/css.inc');
include_once('assets/code.inc');
include_once('assets/functions.inc');
include_once('assets/templates.inc');
include_once('assets/info.inc');
include_once('assets/htaccess.inc');
include_once('assets/fonts.inc');

class Theme {
  private $tid;
  public $layout;
  public $layoutsByPage;
  private $assets = array();
  private $path;
  private $customThemeFolder;
  public $config;
  public $origin;
  public $modified = FALSE;
  public $isDraft = FALSE;
  public $label;
  private $stylesheets;
  private $colorManager;
  public $lastSavedTime;
  public $lastSavedName;
  public $lastSavedLabel;

  static function factory($theme_name, $location = NULL) {
    $theme = new Theme($theme_name, $location);
    return $theme;
  }

  /**
   * Build a new theme as an import from the filesystem.
   */
  static function factoryImport($theme_name, $location = NULL) {
    $theme = new Theme($theme_name, $location, TRUE);
    return $theme;
  }

  public function __construct($theme_name, $location = NULL, $import = FALSE) {
    themebuilder_log_trace_enter(__METHOD__);
    if (NULL == $theme_name) {
      debug_print_backtrace();
      throw new Exception(t('Theme constructor: parameter theme_name cannot be null.'));
    }

    if ($location) {
      $this->setPath($location);
    }

    $this->theme_name = $theme_name;
    // TODO: This needs to come from the .info file.
    $this->baseStylesheets = array('style.css', 'layout.css');

    $this->colorManager = ColorManager::createFromTheme($this);
    $this->setDefaultThemeAssets();
    $this->load();

    // If we are importing a theme, import any extra data about it that is
    // stored in the filesystem.
    if ($import) {
      $this->importThemeDataFromFilesystem();
    }

    themebuilder_log_trace_exit(__METHOD__);
  }

  /*******************************
   *
   * Basic Accessors and Mutators
   *
   ******************************/

  function getTid() {
    return $this->tid;
  }

  /**
   * Sets the human friendly name of the theme
   */
  function setLabel($label) {
    $this->label = $label;
    $this->assets_new['info']->changeName($label);
  }

  function getLabel() {
    // If there is no label, sets it from the info file,
    // if not there, from the name
    if (!$this->label) {
      $this->label = $this->assets_new['info']->name;
      if (!$this->label) {
        return $this->getName();
      }
    }
    return $this->label;
  }

  /**
   * Returns the origin of this theme.  The origin is the theme from which this
   * theme was cloned.  If the theme was never cloned, the origin will be ''.
   *
   * @return string
   *   The name of the theme from which this theme was cloned, if known.
   */
  public function getOrigin() {
    return $this->origin;
  }

  /**
   * Returns the name of this theme.
   *
   * @return string
   *   The name of the theme represented by this instance.
   */
  public function getName() {
    return $this->theme_name;
  }

  /**
   * Returns the name of this theme.
   *
   * @return string
   *   The name of the theme represented by this instance.
   */
  public function setName($name) {
    if ($this->theme_name) {
      $this->assets_new['template_php']->renameFunctions($this->theme_name, $name);
    }
    $this->theme_name = $name;
  }

  /**
   * Returns the path to the theme associated with this instance.  If the theme
   * exists, this returns the path to the existing theme.  If the theme does not
   * yet exist, it will return a path based on whether this instance represents
   * a draft theme or not.
   *
   * @return string
   *   The path to the theme associated with this instance.
   */
  public function getPath() {
    return $this->path;
  }

  /**
   * Returns the relative path to the theme associated with this instance
   * (within the themebuilder custom themes directory).
   */
  private function getRelativePath() {
    // TODO: This assumes that the theme is located within the top level of the
    // custom themes directory. If that changes, revisit this logic.
    return array_pop(explode('/', $this->path));
  }

  function setPath($path) {
    $this->path = $path;
  }

  /**
   * Get a list of css files associated with the base theme.
   *
   * @return array
   *   A list of the theme's stylesheets, relative to the theme directory.
   */
  public function getStylesheets() {
    return $this->baseStylesheets;
  }

  public function getCustomCSSProperties() {
    $assets = $this->assets['custom_css']->getAllProperties();
    $assets = $assets[CssAsset::CUSTOM];
    $other_assets = $this->assets['palette_css']->getAllProperties();
    $other_assets = $other_assets[CssAsset::CUSTOM];

    foreach ($other_assets as $selector => $properties) {
      if (isset($assets[$selector])) {
	$assets[$selector] =  array_merge($assets[$selector], $properties);
      }
      else {
	$assets[$selector] = $properties;
      }
    }
    return $assets;
  }

  /**
   * Returns the version of the theme, as identified in the .info file
   * associated with the theme.
   *
   * @return
   *   The version string.
   */
  public function getVersion() {
    return $this->assets_new['info']->__get('version');
  }

  /**
   * Returns the marketplace version of the theme.
   *
   * This comes from the theme's .info file, if the theme was derived from a
   * theme in the Acquia marketplace; otherwise, it will be NULL.
   *
   * @return
   *   The version string.
   */
  public function getMarketplaceVersion() {
    return $this->assets_new['info']->get('marketplace_version');
  }

  /**
   * Returns information about the theme's author.
   *
   * @return <array>
   *   An associative array with the following keys:
   *   - name: The author's username.
   *   - uid: The author's user id on the Gardens site.
   */
  public function getAuthorInfo() {
    return $this->assets_new['info']->__get('author');
  }

  /*******************************
   *
   * Methods relating to asset manipulation
   *
   ******************************/

  /**
   * This creates named assets which every theme has.
   */
  public function setDefaultThemeAssets() {
    themebuilder_log_trace_enter(__METHOD__);
    // This keeps the function from getting called multiple times.
    if ($this->assets) {
      themebuilder_log_trace_exit(__METHOD__);
      return;
    }

    timer_start(__METHOD__ . '_load1');
    $this->assets['custom_css'] = new CssAsset($this);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load1'));
    timer_start(__METHOD__ . '_load2');
    $this->assets['palette_css'] = new PaletteCssAsset($this);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load2'));
    timer_start(__METHOD__ . '_load3');
    $this->assets_new['advanced_css'] = AdvancedCssAsset::loadFromFile($this->getPath() . '/advanced.css');
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load3'));
    timer_start(__METHOD__ . '_load4');
    $this->assets_new['template_php'] = FunctionAssetNew::loadFromFile($this->getPath() . '/template.php');
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load4'));
    timer_start(__METHOD__ . '_load5');
    $this->assets_new['htaccess'] = HtaccessAsset::loadFromFile($this->getPath() . '/.htaccess');
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load5'));
    timer_start(__METHOD__ . '_load6');
    $this->assets['font_faces'] = new FontFacesAsset($this);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load6'));

    foreach ($this->assets as &$asset) {
      $asset->loadAsset();
    }

    $this->assets_new['info'] = ThemebuilderInfo::loadFromFile($this->getPath() . '/' . $this->getName() . '.info');

    // Set the current user as theme author, overriding any previous value.
    $author_info = array();
    $author_info['uid'] = $GLOBALS['user']->uid;
    $author_info['name'] = isset($GLOBALS['user']->name) ? $GLOBALS['user']->name : '';
    $this->assets_new['info']->set('author', $author_info);

    // If this theme doesn't have a global layout set already, set one.
    if (!$this->getLayout()) {
      $this->setLayout($this->assets_new['info']->layout['<global>']);
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Loads all assets associated with this theme from the database.
   */
  private function loadAssetsFromDB() {
    themebuilder_log_trace_enter(__METHOD__);
    // TODO: Move this hardcoded list and its counterpart in
    // Theme::setDefaultThemeAssets(), perhaps to a static method.
    $name_map = array (
      'custom.css' => 'custom_css',
      'palette.css' => 'palette_css',
      'advanced.css' => 'advanced_css',
      $this->getName() . '.info' => 'info',
      'template.php' => 'template_php',
      'font-families.css' => 'font-families.css',
    );
    timer_start(__METHOD__ . '_query');
    $result = db_select('themebuilder_assets', 't')
    ->fields('t', array('aid', 'type', 'name'))
    ->condition('tid', $this->tid)
    ->execute();
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_query'));
    foreach ($result as $asset_record) {
      $asset = new $asset_record->type($this);
      $asset->aid = $asset_record->aid;
      $asset->loadAsset();
      if (isset($name_map[$asset_record->name])) {
        $asset_name = $name_map[$asset_record->name];
        if (!empty($asset_name)) {
          $this->assets[$asset_name] = $asset;
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Load default assets, unless there is a version saved in the database.
   */
  public function loadAssets() {
    themebuilder_log_trace_enter(__METHOD__);
    if (!$this->tid) {
      $this->setDefaultThemeAssets();
    } else {
      $this->loadAssetsFromDB();
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  public function getLayout($url = null, $nodefault = false) {
    if (!$url) {
      return $this->layout;
    }
    else {
      if ($url == '/') {
        $url = '<front>';
      }
      if (isset($this->assets_new['info']->layout[$url])) {
        return $this->assets_new['info']->layout[$url];
      }
      elseif (!$nodefault) {
        return $this->layout;
      }
      else {
        return ''; // if there's no page-specific layout set, and we
                   // want to know that
      }
    }
  }

  /**
   * Return a list of saved layouts as an array.
   */
  public function getLayouts() {
    return $this->assets_new['info']->get('layout');
  }

  public function setLayout($layout, $url = null) {
    if (!$url || $url == '<global>') {
      if (!$layout){
        throw new Exception("Can't delete the global layout");
      }
      $this->layout = $layout;
      $this->assets_new['info']->layout['<global>'] = $layout;
    }
    else {
      if ($url == '/') {
        $url = '<front>';
      }
      if (!$layout || $layout == ''){
        if (isset($this->assets_new['info']->layout[$url])){
          unset($this->assets_new['info']->layout[$url]);
        }
      }
      else {
        $this->assets_new['info']->layout[$url] = $layout;
      }
    }
  }

  function setCSSProperty($selector, $property_name, $value) {
    themebuilder_log_trace_enter(__METHOD__);
    // If we've been passed a color index, we need to handle whether it
    // belongs in custom.css or palette.css.

    switch ($this->colorManager->indexType($value)) {
      case ColorManager::PALETTE_INDEX:
        // Palette colors go in palette.css, as indexes, and are removed from
        // custom.css.
        if ($palette_css = $this->assets['palette_css']) {
          $palette_css->setProperty($selector, $property_name, $value);
        }
        if ($custom_css = $this->assets['custom_css']) {
          $custom_css->removeProperty($selector, $property_name);
        }
        break;
      case ColorManager::CUSTOM_INDEX:
        // Custom colors go in custom.css, as hex codes, and are removed from
        // palette.css.
        if ($palette_css = $this->assets['palette_css']) {
          $palette_css->removeProperty($selector, $property_name);
        }
        if ($custom_css = $this->assets['custom_css']) {
          $color = $this->colorManager->paletteIndexToHex($value);
          $custom_css->setProperty($selector, $property_name, '#' . $color);
        }
        break;
      default:
        // We were passed something besides a color index. It goes straight to
        // custom.css.
        if ($custom_css = $this->assets['custom_css']) {
          $custom_css->setProperty($selector, $property_name, $value);
        }
        break;
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Causes the specified custom CSS property to be removed.
   *
   * @param {String} $selector
   *   The selector.
   * @param {String} $property_name
   *   The name of the property.
   */
  function removeCSSProperty($selector, $property_name) {
    themebuilder_log_trace_enter(__METHOD__);
    if ($palette_css = $this->assets['palette_css']) {
      $palette_css->removeProperty($selector, $property_name);
    }
    if ($custom_css = $this->assets['custom_css']) {
      $custom_css->removeProperty($selector, $property_name);
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  function setAdvancedCss($value) {
    $this->assets_new['advanced_css']->setContents($value);
  }

  function getColorManager() {
    return $this->colorManager;
  }

  /**
   * Set the palette associated with this theme.
   *
   * @param <mixed> $palette
   *   Either a Palette object, or a palette id.
   */
  function setPalette($palette) {
    if (!is_a($palette, 'Palette')) {
      $palette = Palette::createFromPaletteId($palette);
    }
    $this->colorManager->setPalette($palette);
    $paletteCssAsset = $this->assets['palette_css'];
    $paletteCssAsset->setPalette($palette);
    $this->assets_new['info']->changeColorset($palette);
  }

  /**
   * Change a given palette swatch, or add it to the palette.
   *
   * @param int $palette_id
   * @param stdClass $state
   */
  function setColor($palette_id, $state) {
    $colorsetToRefresh = $this->colorManager->setColor($palette_id, $state);
    if ($colorsetToRefresh) {
      $this->assets_new['info']->changeColorset($colorsetToRefresh);
    }
  }

  /**
   * Get settings from the theme's info file.
   *
   * Returns a single setting value from the theme's info file if a key is
   * provided. Otherwise, returns all the settings as key-value pairs. If the
   * theme's info file has no settings, returns an empty array.
   *
   * @param string|null $key
   *   Key of the setting to return.
   *
   * @return string|array
   *   Value (string|array) of the setting if $key is provided.
   *   The whole settings array if $key is not provided.
   *   Empty array if theme's info file has no settings.
   */
  public function getThemeSetting($key = NULL) {
    $settings = $this->assets_new['info']->get('settings');
    if (isset($settings)) {
      if ($key !== NULL && isset($settings[$key])) {
        return $settings[$key];
      }
      // Return all the settings if no key was passed in.
      return $settings;
    }
    return array();
  }

  public function setThemeSetting($key, $value) {
    $this->assets_new['info']->changeSetting($key, $value, $this);
  }

  /**
   * Save the value of a nested setting.
   *
   * @param string $key
   *   Key of the setting to set.
   * @param array $parents
   *   Array of parents to use for nesting the $value under $key.
   * @param $value
   *   Value of the setting to set.
   */
  public function setNestedThemeSetting($key, $parents, $value) {
    if (!empty($parents)) {
      $settingsValue = $this->getThemeSetting($key);
      if (!is_array($settingsValue)) {
        $settingsValue = array();
      }
      drupal_array_set_nested_value($settingsValue, $parents, $value, TRUE);
      $this->assets_new['info']->changeSetting($key, $settingsValue, $this);
    }
    else {
      $this->assets_new['info']->changeSetting($key, $value, $this);
    }
  }

  /*******************************
   *
   * General state related methods
   *
   ******************************/

  /**
   * Checks if this theme has been loaded from the DB, or has been marked as a draft.
   * In either case, it would not be a base theme.
   *
   * @return boolean
   *
   */
  function isBaseTheme() {
    $base_theme = $this->getBaseThemeName();
    if (!empty($base_theme)) {
      return $base_theme == $this->getName();
    }
    // This theme is so old that it doesn't have a 'base_theme'
    // field in the .info file.  It simply can't be a base theme
    // because the base themes are always up to date.
    return FALSE;
  }

  /**
   * Tests to determine if the theme represented by this instance exists.  The
   * theme is considered to exist if the theme directory and .info file exist.
   *
   * @return boolean
   *   TRUE if the theme exists, false otherwise.
   */
  public function exists() {
    themebuilder_log_trace_enter(__METHOD__);
    //@TODO: Add check for info file?
    //&& is_file($this->getPath() . '/' . $this->getName() . '.info.php'))
    return is_dir($this->getPath());
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Indicates if the theme has been modified since the last save.  Note that this
   * only applies to a theme used during an edit session.  The modified flag is
   * cleared when the theme is saved.
   *
   * @return boolean
   *   TRUE indicates the theme has been modified; false otherwise.
   */
  public function isModified() {
    return $this->modified;
  }

  /**
   * Indicates whether this is an untitled theme or not.
   *
   * @return {Boolean}
   *   TRUE if this theme is untitled; FALSE otherwise
   */
  public function isUntitled() {
    return empty($this->lastSavedName);
  }

  /**
   * Change the modified flag to TRUE for this theme.
   */
  public function modify() {
    if (!$this->isModified()) {
      $this->modified = TRUE;
    }
  }

  /**
   * Indicates whether this theme is a draft.
   * Since the draft is created, we can just tell the Theme it is a draft.
   *
   * @return boolean
   *   TRUE if this instance represents a draft; FALSE otherwise.
   */
  public function isDraft() {
    return $this->isDraft;
  }



  /*******************************
   *
   * Database persistence methods
   *
   *******************************/

  /**
   * Loads the current theme's data from the database by the theme_name.
   *
   * @param {string} $name
   *   The theme name.
   * @return {PDOStatement}
   *   The result set.
   */
  public static function loadByName($name) {
    $result = db_select('themebuilder_themes', 't')
      ->fields('t', array('tid', 'label', 'origin', 'layout', 'modified'))
      ->condition('theme_name', $name)
      ->orderBy('t.tid', 'DESC')
      ->range(0, 1)
      ->execute();
    return $result;
  }

  /**
   * Loads the current theme's data from the database by the tid
   */
  public function loadById($tid) {
    $result = db_query("SELECT tid, theme_name, label, origin, layout, modified FROM {themebuilder_themes} WHERE tid = ':tid'",
      array(':tid' => $tid));
    return $result;
  }

  /**
   * Loads the Theme data from the DB.
   * Also initializes the assets
   */
  public function load() {
    themebuilder_log_trace_enter(__METHOD__);
    if ($this->tid) {
      $result = $this->loadById($this->tid);
    } elseif ($this->getName()) {
      $result = $this->loadByName($this->getName());
    }

    if (!count($result)) {
      return;
    }

    foreach ($result as $row) {
      foreach ($row as $k => $v) {
        $this->$k = $v;
      }
    }
    //Reloads the assets from the DB.
    $this->loadAssets();
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Persists the theme's data to the database.
   * Also goes through the assets and tells them to persist their data.
   *
   * @throws PDOException, Exception
   */
  public function save() {
    themebuilder_log_trace_enter(__METHOD__);
    $theme_info = new stdClass();
    $theme_info->theme_name = $this->getName();
    $theme_info->label = $this->getLabel();
    $theme_info->layout = $this->layout;
    $theme_info->origin = $this->origin;
    $theme_info->modified = $this->modified;
    $theme_info->basetheme = $this->getBaseThemeName();
    $theme_info->version = $this->getVersion();
    $theme_info->marketplace_version = $this->getMarketplaceVersion();

    // Make sure we have a tid before saving or we will end up with a
    // separate copy of the theme in the themebuilder_themes and
    // themebuilder_assets tables for each save operation.
    if (empty($this->tid)) {
      $tid = db_select('themebuilder_themes', 't')
        ->fields('t', array('tid'))
        ->condition('theme_name', $this->getName())
        ->orderBy('t.tid', 'DESC')
        ->range(0, 1)
        ->execute()
        ->fetchField();
      $this->tid = intval($tid);
    }
    if (!empty($this->tid)) {
      $theme_info->tid = $this->tid;
      timer_start(__METHOD__ . '_wr1');
      $result = drupal_write_record('themebuilder_themes', $theme_info, 'tid');
      themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_wr1'));
    } else {
      timer_start(__METHOD__ . '_wr2');
      // May throw PDOException
      $result = drupal_write_record('themebuilder_themes', $theme_info);
      themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_wr2'));
    }

    if ($result == SAVED_NEW) {
      $this->tid = $theme_info->tid;
    }
    elseif ($result != SAVED_UPDATED ) {
      throw new Exception("ERROR updating / insert theme record.  No reason provided."); //This is because Drupal developers think ErrorHandling is for squares
    }

    timer_start(__METHOD__ . '_for1');
    foreach($this->assets as $asset) {
      $asset->saveAsset();
    }
    themebuilder_log_code_time(__METHOD__, __LINE__ - 2, timer_stop(__METHOD__ . '_for1'));
    themebuilder_log_trace_exit(__METHOD__);
  }

  function deleteTheme() {
    themebuilder_log_trace_enter(__METHOD__);
    if ($this->isBaseTheme()) {
      throw new Exception('Cowardly refusing to delete a base theme');
    }
    if ($this->tid) {
      $result = db_delete('themebuilder_themes', array())
      ->condition('tid', $this->tid)
      ->execute();
      //Also remove the database entries, and make the assets do the same
      foreach ($this->assets as $asset) {
        $asset->deleteAsset();
      }
    }

    //wipe out the dir where it is located.
    $this->deleteDirectory($this->getPath(), TRUE);

    themebuilder_log_trace_exit(__METHOD__);
  }


  /*******************************
   *
   * File handling and cloning
   *
   ******************************/


  /**
   * Generates the generated assets.  If output_directory is provided, will use an alternate dir.
   */
  function dist($output_directory = NULL) {
    themebuilder_log_trace_enter(__METHOD__);
    if (!$output_directory) {
      $output_directory = $this->getPath();
    }

    foreach ($this->assets as $asset) {
      $asset->renderFile($output_directory);
    }
    $this->assets_new['info']->write($this->getPath() . '/' . $this->getName() . '.info');
    $this->assets_new['template_php']->write($this->getPath() . '/template.php');
    $this->assets_new['advanced_css']->write($this->getPath() . '/advanced.css');

    if ($this->isDraft()) {
      $this->assets_new['htaccess']->write($this->getPath() . '/.htaccess');
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Exports the theme, allowing assets to write any additional files that are needed.
   */
  function export($output_directory = NULL) {
    themebuilder_log_trace_enter(__METHOD__);
    if (!$output_directory) {
      $output_directory = $this->getPath();
    }
    foreach ($this->assets as $asset) {
      $asset->exportAsset($output_directory);
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Imports any theme data that is stored in the filesystem.
   */
  function importThemeDataFromFilesystem() {
    themebuilder_log_trace_enter(__METHOD__);
    // Allow each asset to import its own data.
    foreach ($this->assets as &$asset) {
      $asset->importAsset();
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Clones the specified theme and its assets, without writing to the database.
   *
   * @param string $new_theme_name
   *   The new theme's name
   *
   * @param string $new_location
   *   The path of where the new Theme should reside.
   */
  public function cloneTheme($new_theme_name, $new_location) {
    themebuilder_log_trace_enter(__METHOD__);
    // It would be bad if the user aborted the request and the theme was only
    // partially cloned as a result.
    ignore_user_abort(TRUE);
    $new_theme = clone($this);

    unset($new_theme->tid);
    foreach ($new_theme->assets_new as $k => $asset) {
      $new_theme->assets_new[$k] = clone($asset);
    }
    $new_theme->setName($new_theme_name);
    $new_theme->setPath($new_location);
    $new_theme->origin = $this->getName();
    $new_theme->modified = FALSE;
    $new_theme->setBaseThemeName($this->getBaseThemeName());
    $new_theme->setPalette($this->getColorManager()->palette->id);

    if ($this->isBaseTheme()) {
      // For base themes, set the base palette to be the same as the palette.
      $new_theme->setBasePalette($this->assets_new['info']->__get('palette'));
    }
    else {
      $base_palette = $this->assets_new['info']->__get('base_palette');
      if (empty($base_palette)) {
        // Only set the base palette if it hasn't been set into the theme
        // already.  Otherwise it will be copied from the original theme.
        $palette_name = $this->getPaletteNameFromBaseTheme();
        $new_theme->setBasePalette($this->getOriginalPaletteFromPaletteName($palette_name));
      }
    }

    foreach ($new_theme->assets as $k => $asset) {
      $new_theme->assets[$k] = clone($asset);

      // Clear out the primary key so a new one will be created.
      unset($new_theme->assets[$k]->aid);
      // This will force the asset to reload stuff dependent on the theme.
      $new_theme->assets[$k]->setTheme($new_theme);
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $new_theme;
  }

  /**
   * Copies the theme files associated with this instance into the specified directory.
   *
   * @param $new_location
   *   The directory in which the theme files should be copied.
   */
  public function copyTheme($new_location, $remove_theme_specific_files = TRUE) {
    // On our production servers the theme directory is sometimes not
    // completely copied.  This is rare, but if it happens, try copying the
    // directory contents multiple times to see if the problem clears itself.
    $max_attempts = 3;
    $attempts = 0;
    do {
      $success = FALSE;
      try {
        $this->copyThemeFiles($new_location, $remove_theme_specific_files);
        $success = TRUE;
      }
      catch (Exception $e) {
        ThemeBuilderLog()->error('Failed to copy the theme.', 'Message: ' . $e->getMessage());

        // Is the system too busy?  Let it cool down a bit.  This
        // causes the attempts to copy the theme to be spread out
        // potentially over 12 seconds, hopefully giving the system
        // time to correctly work through the copy.
        sleep(($attempts + 1) * 2);
      }
    }
    while ($success != TRUE && ++$attempts < $max_attempts);
    if (isset($e)) {
      // An exception was thrown.  Report the issue.
      $message = $e->getMessage();
      $message .= ' - success=' . ($success == TRUE ? 'TRUE' : 'FALSE') . ' after ' . $attempts . ' attempts.';
      if ($success == TRUE) {
        ThemeBuilderLog()->error('Theme copy was successful but took multiple attempts', $message);
      }
      else {
        throw new Exception($message);
      }
    }
  }

  public function symlinkBugThrowException($src) {
    ThemeBuilderLog()->gardensError('Themebuilder symlink bug on save.', "$src not a valid theme directory");
    // This should log to php-errors.log with all the verbose info.
    $cookie = array();
    foreach ($_COOKIE as $k => $v) {
      // Omit session cookies from the log.
      if (strpos($k, 'SESS') === FALSE) {
        $cookie[$k] = $v;
      }
    }
    // Only log HTTP headers in $_SERVER, since the rest is not helpful.
    $server = $_SERVER;
    unset ($server['HTTP_COOKIE']);
    foreach ($_SERVER as $k => $v) {
      if (strpos($k, 'HTTP_') !== 0) {
        unset($server[$k]);
      }
    }
    ThemeBuilderLog()->error('Error saving the theme.', "Source: $src.  \$_COOKIE dump:\n" . print_r($cookie, TRUE) . "\n \$_SERVER dump:\n" . print_r($server, TRUE));
    throw new ThemeBuilderException('We\'re sorry, but there was an error saving your recent appearance changes and your theme was reverted to the last version saved.  Our team has been notified of the error and will be investigating a fix ASAP.  Please click Appearance to try again.', THEMEBUILDER_ERROR_RECOVERABLE, array(ThemeBuilderException::HANDLER_ALERT_AND_CLOSE));
  }

  /**
   * Moves this theme to the backup directory in case a theme save fails.  This backup
   * can then be restored so the user's theme is not destroyed.
   *
   * @return
   *   TRUE if the backup was successful; FALSE otherwise.
   */
  public function moveToBackup() {
    themebuilder_log_trace_enter(__METHOD__);
    try {
      $this->backupDir = $this->getBackupDir();
      $theme_path = $this->getPath();
      $this->verifyAndFixDirectoryPerms($theme_path);

      // Move the directory into the backup directory.
      $success = rename($theme_path, $this->backupDir);
    }
    catch (Exception $e) {
      $message = $e->getMessage();
      ThemeBuilderLog()->gardensError('Failed to back up theme directory.', "Error is $message");
      $success = FALSE;
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $success;
  }

  /**
   * Restores the theme from its backup.
   *
   * @return
   *   TRUE if the restore succeeded; FALSE otherwise.
   */
  public function restore() {
    themebuilder_log_trace_enter(__METHOD__);
    $success = FALSE;
    $path = $this->getPath();
    if (isset($this->backupDir) && file_exists($this->backupDir) && isset($path)) {
      $attempt = 1;
      do {
        try {
          if (file_exists($path)) {
            $this->deleteDirectory($path);
          }
	  if (!file_exists($path)) {
	    $this->verifyAndFixDirectoryPerms($this->backupDir);
	    // Move the backup directory back to the theme directory.
	    $success = rename($this->backupDir, $this->getPath());
	  }
	  else {
	    // Failed to delete the theme directory.
	    throw new Exception("Could not delete theme directory ${path}.");
	  }
        }
        catch (Exception $e) {
          $message = $e->getMessage();
          $success = FALSE;
        }
      } while (!$success && $attempt++ <= 3);
    }
    if (!$success && $message) {
      $dir = $this->backupDir;
      ThemeBuilderLog()->gardensError('Failed to restore from theme backup directory', "Directory $dir.  Error is $message");
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $success;
  }

  /**
   * Deletes the backup.
   *
   * @return
   *   TRUE if the delete succeeded; FALSE otherwise.
   */
  public function deleteBackup() {
    $success = TRUE;
    if (isset($this->backupDir) && file_exists($this->backupDir)) {
      $attempt = 1;
      do {
        try {
          $this->deleteDirectory($this->backupDir, TRUE);
          $success = TRUE;
        }
        catch (Exception $e) {
          $message = $e->getMessage();
          $success = FALSE;
          $this->verifyAndFixDirectoryPerms($this->backupDir);
        }
      } while (!$success && $attempt++ <= 3);
    }
    if (!$success && isset($message)) {
      $dir = $this->backupDir;
      $message = $e->getMessage();
      ThemeBuilderLog()->gardensError('Failed to delete the backup directory.', "Directory $dir.  Error is $message");
    }
    return $success;
  }

  /**
   * Creates a theme backup directory for the files that comprise this theme.
   *
   * @return
   *   A string containing the full path to the created backup directory.
   */
  private function getBackupDir() {
    $public_files_path = file_stream_wrapper_get_instance_by_scheme('public')->getDirectoryPath();
    $backup_path = DRUPAL_ROOT . '/' . $public_files_path . '/backup';

    // Make sure the backup directory exists.
    if (!file_exists($backup_path)) {
      @mkdir($backup_path, 0755, TRUE);
    }

    $path =  "${backup_path}/" . $this->getName() . '_' .microtime(TRUE);
    if (file_exists($path)) {
      $this->verifyAndFixDirectoryPerms($path);
      $this->deleteDirectory($path);
    }
    return $path;
  }

  /**
   * Deletes all backups associated with this theme.
   *
   * @param {String} $exclude
   *   Optional.  If provided, the specified backup will be excluded
   *   from the delete operation.
   */
  public function deleteAllBackups($exclude = NULL) {
    $public_files_path = file_stream_wrapper_get_instance_by_scheme('public')->getDirectoryPath();
    $path_prefix =  DRUPAL_ROOT . "/${public_files_path}/backup";
    $name = $this->getName();
    $command = 'cd ' . escapeshellarg($path_prefix) . '; ls -d ' . escapeshellarg($name) . "_* |egrep '^" . escapeshellarg($name) . "_[0-9]+.[0-9]+$'";
    exec($command . ' 2>&1', $output, $result);
    for ($i = 0, $len = count($output); $i < $len; $i++) {
      $dir = "${path_prefix}/${output[$i]}";
      if ($output[$i] == $exclude || $dir == $exclude) {
        continue;
      }
      $this->deleteDirectory($dir, TRUE);
    }
  }

  private function copyThemeFiles($new_location, $remove_theme_specific_files = TRUE) {
    themebuilder_log_trace_enter(__METHOD__);
    $location = $new_location;
    if ($new_location === $this->getPath()) {
      // Cannot copy a theme to itself.  This will absolutely result in a
      // broken theme.  This can happen if the draft theme already exists when
      // the user enters the themebuilder.
      return;
    }
    $path_info = pathinfo($new_location);
    $theme_name = $path_info['basename'];
    if (file_exists($new_location)) {
      $this->deleteDirectory($new_location);
    }
    if (themebuilder_compiler_theme_is_draft($theme_name) && themebuilder_compiler_use_local_directory()) {
      $local_dir = themebuilder_compiler_create_edit_session_directory($theme_name);
      if ($local_dir !== $new_location) {
        $new_location = $local_dir;
      }
    }
    @rmdir($new_location);
    $this->copyDirectory($this->getPath(), $new_location);
    $this->verifyAndFixDirectoryPerms($new_location);

    if ($remove_theme_specific_files) {
      $orig_info_file = $new_location .'/' . $this->getName() .'.info';
      if (file_exists($orig_info_file)) {
        // The .info file must be deleted and regenerated from scratch because the
        // name has to be changed.
        @unlink($orig_info_file);
      }
      $orig_htaccess_file = $new_location .'/.htaccess';
      if (file_exists($orig_htaccess_file)) {
        // .htaccess files should exist only for draft themes, so the theme's
        // dist() method is responsible for writing it.
        @unlink($orig_htaccess_file);
      }
    }

    // Check to see if there are any files missing.
    $images = $default = $style = FALSE;
    $dir = @dir($new_location);
    if ($dir) {
      while(false !== ($entry = $dir->read())) {
        switch ($entry) {
          case 'images':
            $images = TRUE;
            break;
          case 'defaults.css':
            $default = TRUE;
            break;
          case 'style.css':
            $style = TRUE;
            break;
        }
      }
      $dir->close();
      if (!($images && $default && $style)) {
        // The copied theme is incomplete.  Figure out what files the source
        // theme should have copied for the error report.
        $dir_summary = '';
        $dir = @dir($this->getPath());
        while (false !== ($entry = $dir->read())) {
          if (empty($dir_summary)) {
            $dir_summary = "$entry";
          }
          else {
            $dir_summary .= ",$entry";
          }
        }
        $dir->close();
        throw new Exception('GardensError: Incomplete copy of a theme was made copying ' . $this->getPath() . ' to ' . $new_location . '; files=' . $dir_summary);
      }
    }
    if (is_link($location) && !themebuilder_compiler_theme_is_draft($theme_name)) {
      throw new Exception("GardensError: Copy resulted in a symlink for non-draft theme $theme_name (copied from " . $this->getPath() . " to $location).");
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Verifies the specified directory has appropriate permissions, and if not
   * it logs a message and fixes the permissions.
   *
   * @param {String} $dir
   *   The directory.
   *
   * @return {boolean}
   *   TRUE if the permissions are correct or were successfully corrected;
   *   false otherwise.
   */
  private function verifyAndFixDirectoryPerms($dir) {
    $result = TRUE;
    if ((fileperms($dir) & 0777) != 0755) {
      // The permissions are not correct.
      ThemeBuilderLog()->error('The directory permissions are incorrect.', "permissions for directory $dir should be 0750, but are " . decoct(fileperms($dir) & 0777));
      $result = chmod($dir, 0755);
    }
    return $result;
  }

  /**
   * Recursively removes the specified directory.
   *
   * @param {string} $path
   *   The directory to remove.
   * @param {boolean} $asynchronous
   *   If true, the delete will occur asynchronously.
   */
  private function deleteDirectory($path, $asynchronous = FALSE) {
    themebuilder_log_trace_enter(__METHOD__);
    if (empty($path)) {
      themebuilder_log_trace_exit(__METHOD__);
      return;
    }
    // Make the path is an asbolute path.
    if (substr($path, 0, 1) !== '/') {
      $path = DRUPAL_ROOT . "/${path}";
    }
    $command = 'rm -rf ' . escapeshellarg($path);
    $redirect = '2>&1';
    if ($asynchronous) {
      $command .= ' > /dev/null';
      $redirect .= ' &';

      // Since we are doing an asynchronous delete, be careful about
      // the theme list being regenerated as this delete is occuring
      // in the background.  Trouble can be avoided by breaking the
      // theme before this method completes.  Here we simply delete
      // the .info file which takes much less time than deleting the
      // entire theme, and then delete the rest of the theme in the
      // background.
      $info_path = "${path}/" . basename($path) . ".info";
      @unlink($info_path);
    }
    timer_start(__METHOD__ . '_rm');
    exec("${command} ${redirect}", $output, $result);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_rm'));
    if ($result !== 0) {
      // The recursive delete failed.  This should never happen unless
      // someone has inadvertently added files to the directory with
      // incorrect file ownership.  But alas we are using gluster and
      // we do experience the occasional hiccup such as directories
      // without executable permission or a directory with permission
      // "???", indicating gluster is unwell.  We can try to repair
      // this by setting directory permissions and doing the delete
      // operation again.  First log the issue so we know this is
      // happening.
      exec('ls -lR ' . escapeshellarg($path) . ' 2>&1', $ls);
      ThemeBuilderLog()->gardensWarning('Failed to delete theme directory on the first attempt, trying again', "path: $path; error message: " . implode('; ', $output) . '; directory listing: ' . implode('; ', $ls));

      $this->verifyAndFixDirectoryPerms($path);
      $command = 'rm -rf ' . escapeshellarg($path);
      exec($command . ' 2>&1', $output, $result);
    }
    if ($result !== 0) {
      // The recursive delete failed.  Log the issue with specifics on
      // the remaining files and try again.
      exec('ls -lR ' . escapeshellarg($path) . ' 2>&1', $ls);
      ThemeBuilderLog()->gardensError('AN-25212 - Failed to delete theme directory', "path: $path; error message: " . implode('; ', $output) . '; directory listing: ' . implode('; ', $ls));
      // Continue despite the error.
    }
    clearstatcache();
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Recursively copies the source directory to the destination directory.  The
   * destination directory is not deleted before this copy occurs.
   *
   * @param string $src
   *   The path to the source directory.
   * @param <type> $dest
   *   The path to the destination directory.
   * @param array $exclude_patterns
   *   An array of preg expressions to match against file names. If it matches,
   *   file or dir will be ignored.
   */
  private function copyDirectory($src, $dest, $exclude_patterns = NULL) {
    themebuilder_log_trace_enter(__METHOD__);
    if (file_exists($src)) {
      // The src directory may be a link.
      $full_src = realpath($src);
      if (!$full_src) {
        $full_src = realpath(DRUPAL_ROOT . "/$src");
      }
      $src = $full_src;
    }
    // Use a simple cp, for speed, if we don't need to exclude any files.
    if (empty($exclude_patterns)) {
      // Note that cp does not care whether you use a trailing slash or not.
      // And oddly, cp --no-preserve=mode still preserves the mode.

      // It is tempting to combine these execs into a single exec.
      // Don't do it because the status reported only reveals the
      // success or failure of the last command in the exec, not the
      // combined success or failure.  If you really care about
      // detecting failure here (and we do) keep these separate.
      $command = 'cp -Rf ' . escapeshellarg($src) . ' ' .escapeshellarg($dest);
      timer_start(__METHOD__ . '_cp');
      exec($command . ' 2>&1', $output, $copy_status);
      themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_cp'));
      if ($copy_status !== 0) {
        throw new Exception("Failed to copy directory $src to $dest with error code $copy_status and message '" . implode(';', $output) . "'.");
      }

      // We chmod the files to be world-readable because they will end
      // up owned by our uid:gid but need to be readable by
      // Apache.
      $command = 'chmod -R u+w,go+rX ' .escapeshellarg($dest);
      unset($output);
      timer_start(__METHOD__ . '_chmod');
      exec($command . ' 2>&1', $output, $chmod_status);
      themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_chmod'));
      if ($chmod_status !== 0) {
        throw new Exception("Failed to change the file permissions for directory $dest with error code $chmod_status and message '" . implode(';', $output) . "'.");
      }
    }
    else {
      if (!file_exists($dest)) {
        if (!mkdir($dest)) {
          throw new Exception('Failed to create directory ' . $this->getFileSummary($dest));
        }
      }
      foreach (new RecursiveIteratorIterator(new RecursiveDirectoryIterator($src), RecursiveIteratorIterator::SELF_FIRST) as $filename => $file) {
        $relative_path = substr($filename, strlen($src));
        foreach ($exclude_patterns as $pattern) {
          $file_path = $file->getPathName();
          if (preg_match($pattern, $file->getPathName())) {
            continue 2;
          }
        }
        $dirname = $dest . '/' . $relative_path;
        if ($file->isDir()) {
          if (!is_dir($dirname)) {
            // Only create the directory if it doesn't already exist or if it is
            // a file rather than a directory.  This is important for the update
            // method, in which we need to preserve existing images in the
            // customized theme while copying any new images that were added to
            // the base theme.
            @unlink($dirname);
            if (!mkdir($dirname)) {
              throw new Exception('Failed to create directory ' . $this->getFileSummary($dest));
            }
          }
        }
        else {
          if (!copy($filename, $dirname)) {
            throw new Exception("Failed to copy $filename to $dirname. " . $this->getCopySummary($filename, $dirname));
          }
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Provides a summary of the specified file suitable for use in a stack
   * trace.
   *
   * @param $file
   *   The path to the file.
   * @return
   *   A string containing the summary.
   */
  private function getFileSummary($file) {
    $exists = file_exists($file) ? 'TRUE' : 'FALSE';
    $result = "Filename: $file, exists: $exists";
    $permissions = $exists == 'TRUE' ? fileperms($file) : '';
    if (!empty($permissions)) {
      $result = $result . ",permissions: $permissions";
    }
    return $result;
  }

  /**
   * Provides a summary of a copy operation suitable for use in a stack trace.
   *
   * @param $src
   *   The path to the source file
   * @param $dest
   *   The path to the destination directory
   * @return
   *   A string containing the summary.
   */
  private function getCopySummary($src, $dest) {
    $result = 'src: ' . $this->getFileSummary($src) . '; dest: ' . $this->getFileSummary($dest);
    return $result;
  }

  /**
   * Updates the theme with the files associated with the specified base theme.
   *
   * @param $base
   *   The Theme instance that represents the base theme from which the file
   *   updates will be pulled.
   */
  function update($base) {
    themebuilder_log_trace_enter(__METHOD__);

    // Make sure the theme has no duplicate entries.
    $themeCorrector = new ThemeCorrector(DRUPAL_ROOT . '/' . themebuilder_compiler_get_custom_theme_folder(), array('DuplicateInfoEntriesThemeElf'));
    $themeCorrector->fix();

    $updateFilters = $this->getUpdateFilters($base->getName(), $this->getVersion(), $base->getVersion());
    // Copy all of the files except the files that can be customized in the
    // course of customizing the theme.
    $exclude = array(
      '/^\.$/',            // Current directory
      '/^\.\.$/',          // Parent directory
      '/\.svn/',           // subversion directory
      '/advanced\.css$/',  // advanced.css (will keep the custom theme file)
      '/custom\.css$/',    // custom.css (will keep the custom theme file)
      '/palette\.css$/',   // palette.css (will keep the custom theme file)
      '/template\.php$/',  // template.php file (will be regenerated from base)
      '/\.info$/',         // theme's .info file (will be merged with base)
    );
    $this->copyDirectory($base->getPath(), $this->getPath(), $exclude);

    // Update the .info file.  Create a new array representing the .info file
    // contents.
    $baseInfo = ThemeBuilderInfo::loadFromFile($base->getPath() . '/' . $base->getName() . '.info')->getContents();
    $newInfo = array();
    $currentInfo = $this->assets_new['info']->getContents();
    foreach ($baseInfo as $property => $value) {
      $newInfo[$property] = $this->getUpdatedInfoValue($baseInfo, $currentInfo, $property, $updateFilters);
    }
    // Copy any properties from the customized theme that don't appear in the
    // base theme's .info file (such as base palette).
    foreach ($currentInfo as $property => $value) {
      if (!isset($newInfo[$property])) {
        $newInfo[$property] = $value;
      }
    }
    $this->assets_new['info']->setContents($newInfo);

    // Set the theme's palette to the newly updated version. This also sets
    // the palete asset's palette and the color manager's palette.
    $this->setPalette(Palette::createFromThemeInfo($newInfo));
    $palette_asset = $this->getPaletteAsset();
    $palette_asset->updatePalette($base, $updateFilters);

    // Update the template.php.
    $newTemplate = FunctionAssetNew::loadFromFile($base->getPath() . '/template.php');
    $newTemplate->renameFunctions($base->getName(), $this->getName());
    $this->assets_new['template_php'] = $newTemplate;

    // Update the custom css
    $css_asset = $this->assets['custom_css'];
    $all_properties = $css_asset->getAllProperties();
    $original_properties = $all_properties[CssAsset::CUSTOM];
    foreach ($updateFilters as $filter) {
      if (!isset($custom_properties)) {
        $custom_properties = $original_properties;
      }
      $custom_properties = $filter->updateCustomizedCss($custom_properties);
    }
    if (isset($custom_properties)) {
      // Clear all of the original custom css rules.
      foreach ($original_properties as $selector => $rules) {
        foreach ($rules as $property => $value) {
          $css_asset->removeProperty($selector, $property);
        }
      }
      // Add all of the modified custom css rules.
      foreach ($custom_properties as $selector => $rules) {
        foreach ($rules as $property => $value) {
          $css_asset->setProperty($selector, $property, $value);
        }
      }
      $css_asset->renderFile($this->getPath());
    }

    // Render the updated theme files.
    $this->dist();
    $this->modify();

    // Clear the theme registry.
    drupal_theme_rebuild();
    // Clear cached .info data.
    system_rebuild_theme_data();
    system_list_reset();
    list_themes(TRUE);

    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Returns a set of ThemeUpdate instances that should be used to update a
   * custom theme derived from the specified base theme from the specified old
   * version to the specified new version.  Only the relevant ThemeUpdates
   * will be returned, and the instances will be arranged in the order in
   * which they should be executed.
   *
   * @param {String} $baseThemeName
   *   The name of the base theme associated with the theme being updated.
   * @param {String} $oldVersion
   *   The current version of the custom theme.
   * @param {String} $newVersion
   *   The current version of the base theme.  The update will update the
   *   custom theme from its current version to the version of the current
   *   base theme.
   * @return
   *   An ordered array of ThemeUpdate instances that are relevant for this
   *   update.
   */
  function getUpdateFilters($baseThemeName, $oldVersion, $newVersion) {
    $updates = array();
    $path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'themebuilder_compiler');
    $update_path = "$path/theme_updates/";
    require_once("$update_path/ThemeUpdate.inc");
    $index = 1;
    do {
      $classname = $baseThemeName . 'Update' . $index;
      $update_include = $update_path . $classname . '.inc';
      $update_include_exists = is_readable($update_include);
      if (is_readable($update_include)) {
        require_once($update_include);
        $update = new $classname($baseThemeName, $oldVersion, $newVersion);
        if ($update->isValid()) {
          // This update instance is required for the current theme update.
          $updates[] = $update;
        }
      }
      $index++;
    }
    while ($update_include_exists);
    return $updates;
  }

  /**
   * Returns the appropriate value for the specified property.  This private
   * method is used during the update of the .info file.  The idea is to go
   * through the new .info file and populate from that a new .info file.  The
   * only items that the customized theme should override are the name, layout
   * information, palette, and the part of the settings array that stores the
   * default logo and favicon path.
   *
   * @param $baseInfo
   *   The .info contents from the base theme.
   * @param $currentInfo
   *   The .info contents from the customized theme.
   * @param $property
   *   The property for which an appropriate value should be returned.
   * @param $updateFilters
   *   The update filters that apply to this update.
   *
   * @return
   *   The value for the specified property that represents the proper merge
   *   between the base theme and customized theme.
   */
  private function getUpdatedInfoValue($baseInfo, $currentInfo, $property, $updateFilters) {
    themebuilder_log_trace_enter(__METHOD__);
    $value = $baseInfo[$property];
    switch ($property) {
      case 'name':
      case 'layout':
        // If there is no name or layout property in the custom theme, use
        // the corresponding property from the base theme.
        $newValue = $this->mergeInfoValue($baseInfo, $currentInfo, $property);
        break;
      case 'palette':
        // Handle any palette color changes that are specified in the update.
        $newValue = $this->replacePaletteInfoValue($baseInfo, $currentInfo, $updateFilters);
        break;
      case 'settings':
        // The settings array should generally be inherited from the base
        // theme, but if the user has uploaded a logo or favicon for this
        // theme, those need to be preserved.
        $newValue = $value;
        foreach (array('default_logo_path', 'default_favicon_path') as $setting) {
          if (!empty($currentInfo[$property][$setting])) {
            $newValue[$setting] = $currentInfo[$property][$setting];
          }
        }
        break;
      default:
        $newValue = $value;
        break;
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $newValue;
  }

  /**
   * Helper method for getUpdatedInfoValue. For a given property, merges new
   * values from the base theme into the current theme, without overwriting
   * existing values.
   */
  private function mergeInfoValue($baseInfo, &$currentInfo, $property) {
    themebuilder_log_trace_enter(__METHOD__);
    // If the property does not exist in the customized theme, use the base
    // theme's value for that property.
    if (empty($currentInfo[$property])) {
      $newValue = $baseInfo[$property];
    }
    // If the property does exist in the customized theme, merge in any new
    // information from the base theme, while leaving existing info intact.
    else {
      $newValue = $currentInfo[$property];
      if (is_array($newValue)) {
        // The value is an array, so we need to go line by line to make
        // sure all of the new additions in the base theme are carried
        // into the customized theme.  Only do this for properties the
        // customized theme does not already have.
        foreach ($baseInfo[$property] as $p => $v) {
          if (empty($newValue[$p])) {
            // The customized theme doesn't have property $p.  Add it
            // here.
            $newValue[$p] = $v;
          }
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $newValue;
  }

  /**
   * Helper method for getUpdatedInfoValue. Uses the palette mapping specified
   * in the update filters to make any needed changes to the existing palette.
   *
   * @param <array> $baseInfo
   * @param <array> $currentInfo
   * @param <array> $updateFilters
   */
  private function replacePaletteInfoValue($baseInfo, $currentInfo, $updateFilters) {
    $newValue = array();
    // If we don't have a palette at all, use the palette from $baseInfo.
    if (empty($currentInfo['palette'])) {
      $newValue = $baseInfo['palette'];
    }
    // If we do have a palette, use it, and merge in anything new from
    // the base theme. Also apply any mappings.
    else {
      $newValue = $this->mergeInfoValue($baseInfo, $currentInfo, 'palette');
      foreach ($updateFilters as $filter) {
          $newValue = $filter->updatePaletteInfo($newValue);
      }
      // Compare the mapped palette to the $baseInfo palette.
      $identical = TRUE;
      $colors = Palette::getIndexes();
      foreach ($baseInfo['palette'] as $key => $value) {
        if (in_array($key, $colors)) {
          if (strtolower($baseInfo['palette'][$key]['hex']) !== strtolower($newValue[$key]['hex'])) {
            $identical = FALSE;
          }
        }
      }
      // If the palette colors are different from the new base palette,
      // create a new UUID and name. This keeps the old palette from overriding
      // the new one in palette lists, and means the user can switch to it.
      if (!$identical) {
        $newValue['id'] = uuid();
        $newValue['name'] = "Legacy";
      }
    }
    return $newValue;
  }

  /**
   * Copies an image file into the theme's image directory.
   *
   * @param StdClass $file (from file_save_upload)
   * @return string The absolute path to the created file.
   */
  function addImage($file) {
    themebuilder_log_trace_enter(__METHOD__);
    $wrapper = file_stream_wrapper_get_instance_by_scheme('customthemes');
    $imagePath = $this->getRelativePath() . '/images';
    $destinationUri = 'customthemes://' . $imagePath;
    $destination = drupal_realpath($destinationUri);
    file_prepare_directory($destination, FILE_CREATE_DIRECTORY);
    $file = file_unmanaged_copy($file->uri, $destinationUri, FILE_EXISTS_RENAME);
    // Assemble a filename for this URI. drupal_realpath() would work, but will
    // resolve symlinks, which we don't want.
    $filename = basename($file);
    $directory = $wrapper->getDirectoryPath();
    themebuilder_log_trace_exit(__METHOD__);
    return DRUPAL_ROOT . '/' . $directory . '/' . $imagePath . '/' . $filename;
  }

  /**
   * Copies a resource into the theme's directory.
   *
   * The themebuilder_resources module can provide resources (such as font
   * files, images, or icons) that are available while Themebuilder is open.
   * For those resources to be available to the theme's CSS after Themebuilder
   * is closed, they have to be copied to the theme directory itself.
   *
   * @param string $source
   *   The source path for the resource, either relative to the Drupal root,
   *   or absolute.
   * @param string $destination
   *   The destination for the resource, relative to the theme path.
   */
  function addResource($source, $destination, $overwrite = FALSE) {
    // Ensure the $destination path's parent directory exists; copyDirectory()
    // won't create target directory structure past one level.
    $destination = $this->getPath() . '/' . $destination;
    $parent = dirname($destination);
    if (!file_exists($parent)) {
      mkdir($parent, 0755, TRUE);
    }
    // Copy the resource to the theme if it's not there already, or if we've
    // been instructed to overwrite it.
    if ($overwrite || !file_exists($destination)) {
      $this->copyDirectory($source, $destination);
    }
  }

  /**
   * Save a cached version of the theme's palette.css in its directory, if one
   * doesn't exist.  This does NOT use Drupal's caches.  Note that this method
   * is destructive to the palette asset.
   */
  public function seedCache() {
    themebuilder_log_trace_enter(__METHOD__);
    $this->assets['palette_css']->loadProperties();
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Returns the base theme name, which is the original theme this theme was
   * created from.  This is important in case the base theme is updated, this
   * allows the themebuilder to ascertain an update is available.
   *
   * @return
   *   A string containing the base theme name associated with this theme.
   */
  function getBaseThemeName() {
    $base_theme = $this->assets_new['info']->__get('base_theme');
    if (empty($base_theme)) {
      $base_theme = themebuilder_compiler_get_base_theme_name_from_screenshot($this);
    }
    return $base_theme;
  }

  /**
   * Sets the specified theme name as the base theme.  This information will
   * be recorded in the .info file of the associated theme.
   *
   * @param $theme_name
   *   The name of the theme that represents the base theme.
   */
  function setBaseThemeName($theme_name) {
    $this->assets_new['info']->set('base_theme', $theme_name);
  }

  /**
   * Returns the base palette, which is the original palette this theme
   * used.  This is important during a merge so the generated section of the
   * palette.css file can be compared to the fully customized version so the
   * appropriate color customizations can be applied
   *
   * @return
   *   A Palette instance containing the base palette colors associated with
   *   this theme.
   */
  function getBasePalette() {
    if ($this->isBaseTheme()) {
      // Base themes do not have the base palette enumerated in the info file.
      // Use the palette from the info file instead.
      $base_palette = $this->assets_new['info']->__get('palette');
    }
    else {
      // When the theme is cloned, a base_palette entry is added to the .info
      // file.  Use that.
      $base_palette = $this->assets_new['info']->__get('base_palette');
    }
    if (empty($base_palette)) {
      $palette_name = $this->getPaletteNameFromBaseTheme();
      $base_palette = $this->getOriginalPaletteFromPaletteName($palette_name);
    }
    return $base_palette;
  }

  /**
   * This method should only be used in the case that the base palette
   * was not set in the .info file.
   *
   * @return
   *   The name of the palette associated with the base theme.
   */
  private function getPaletteNameFromBaseTheme() {
    $map = array(
      'broadway' => 'Broadway',
      'builderbase' => 'Minima',
      'campaign' => 'Campaign',
      'impact' => 'Impact',
      'kenwood' => 'Kenwood',
      'sonoma' => 'Sonoma',
      'sparks' => 'Sparks',
    );
    $palette_name = $map[$this->getBaseThemeName()];
    return $palette_name;
  }

  /**
   * Returns an array populated with the colors from the original palette that
   * is associated with the specified palette name.  This is legacy, and used
   * only in themes that were created from one of the original themebuilder
   * releases for which the original palette colors were not written into the
   * .info file.
   *
   * @param {String} palette_name
   *   The name of the original palette.
   * @return
   *   An array containing the colors.
   */
  private function getOriginalPaletteFromPaletteName($palette_name) {
    $palettes = array (
      'Broadway' => array('id' => '8A0AA0F7-E1CB-5AD0-04E2F8C20CF48645',
                  'name' => 'Broadway',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '333333',
                           'name' => 'Charcoal'),
                  'a' => array('hex' => 'EEEEEE',
                       'name' => 'Type'),
                  'b' => array('hex' => '93A9C0',
                       'name' => 'Light Blue'),
                  'c' => array('hex' => 'C2BF70',
                       'name' => 'Muted Green'),
                  'd' => array('hex' => 'E0DB52',
                       'name' => 'Bright Green'),
                  'e' => array('hex' => 'AAAAAA',
                       'name' => 'Grey')),
      'Minima' => array('id' => '8A0AA183-A2BD-21A5-8C16817D4AC0B9C6',
                  'name' => 'Minima',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '000000',
                           'name' => 'Black'),
                  'a' => array('hex' => 'FF7800',
                       'name' => 'Orange'),
                  'b' => array('hex' => '333333',
                       'name' => 'Charcoal'),
                  'c' => array('hex' => '666666',
                       'name' => 'Grey Tint 1'),
                  'd' => array('hex' => '999999',
                       'name' => 'Grey Tint 2'),
                  'e' => array('hex' => 'EEEEEE',
                       'name' => 'Grey Tint 3')),
      'Campaign' => array('id' => '8A0AA107-EE01-35ED-3936AFE868E23B7A',
                  'name' => 'Campaign',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '444444',
                           'name' => 'Charcoal'),
                  'a' => array('hex' => 'cd1069',
                       'name' => 'Dark Purple'),
                  'b' => array('hex' => 'A42470',
                       'name' => 'Purple'),
                  'c' => array('hex' => 'E1267C',
                       'name' => 'Light Purple'),
                  'd' => array('hex' => '4DABF5',
                       'name' => 'Light Blue'),
                  'e' => array('hex' => '212121',
                       'name' => 'Dark Grey')),
      'Impact' => array('id' => '8A0AA135-0AEB-F0C9-F773BC173B064C48',
                'name' => 'Impact',
                'author' => 'Acquia',
                'white' => array('hex' => 'ffffff',
                         'name' => 'White'),
                'black' => array('hex' => '121212',
                         'name' => 'Charcoal'),
                'a' => array('hex' => '62a734',
                     'name' => 'Green'),
                'b' => array('hex' => '00a5ff',
                     'name' => 'Light Green'),
                'c' => array('hex' => 'cccccc',
                     'name' => 'Light Grey'),
                'd' => array('hex' => 'd3fec1',
                     'name' => 'Light Blue'),
                'e' => array('hex' => 'f5f5f5',
                     'name' => 'Grey')),
      'Kenwood' => array('id' => '8A0AA164-ECCA-0E4E-BE824CA8DF22D011',
                  'name' => 'Kenwood',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '333333',
                           'name' => 'Charcoal'),
                  'a' => array('hex' => '5D0604',
                       'name' => 'Ruby Red'),
                  'b' => array('hex' => '1D1D2A',
                       'name' => 'Dark Blue'),
                  'c' => array('hex' => 'EBEADE',
                       'name' => 'Tan'),
                  'd' => array('hex' => 'E6DBC8',
                       'name' => 'Light Tan'),
                  'e' => array('hex' => 'FFFDFB',
                       'name' => 'Offwhite')),
      'Sonoma' => array('id' => '8A0AA1B2-D33B-10DC-EE9ADC855F128DF1',
                  'name' => 'Sonoma',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '333333',
                           'name' => 'Charcoal'),
                  'a' => array('hex' => '2C70C5',
                       'name' => 'Primary Blue'),
                  'b' => array('hex' => '8BBFFF',
                       'name' => 'Secondary Blue'),
                  'c' => array('hex' => '2E496B',
                       'name' => 'Heading Dark Blue'),
                  'd' => array('hex' => '666666',
                       'name' => 'Light Grey'),
                  'e' => array('hex' => 'EEEEEE',
                       'name' => 'Lighter Grey')),
      'Sparks' => array('id' => '8A0AA1C2-0FBD-DDF0-3F517B0B188D29A7',
                  'name' => 'Sparks',
                  'author' => 'Acquia',
                  'white' => array('hex' => 'FFFFFF',
                           'name' => 'White'),
                  'black' => array('hex' => '333333',
                           'name' => 'Charcoal'),
                  'a' => array('hex' => '7B7B8B',
                       'name' => 'Slate Blue'),
                  'b' => array('hex' => '707969',
                       'name' => 'Olive Green'),
                  'c' => array('hex' => '1D1D2A',
                       'name' => 'Dark Charcoal'),
                  'd' => array('hex' => '666666',
                       'name' => 'Light Grey'),
                  'e' => array('hex' => '000000',
                       'name' => 'Black')),

    );
    $palette = $palettes[$palette_name];
    return $palette;
  }

  /**
   * Sets the specified palette as the base palette.  This information will be
   * recorded in the .info file of the associated theme.
   *
   * @param $palette
   *   The palette that represents the original palette.
   */
  function setBasePalette($palette) {
    $this->assets_new['info']->set('base_palette', $palette);
  }

  function getPaletteAsset() {
    return $this->assets['palette_css'];
  }
}

/**
 * Generates a Universally Unique IDentifier, version 4.
 *
 * RFC 4122 (http://www.ietf.org/rfc/rfc4122.txt) defines a special type of Globally
 * Unique IDentifiers (GUID), as well as several methods for producing them. One
 * such method, described in section 4.4, is based on truly random or pseudo-random
 * number generators, and is therefore implementable in a language like PHP.
 *
 * We choose to produce pseudo-random numbers with the Mersenne Twister, and to always
 * limit single generated numbers to 16 bits (ie. the decimal value 65535). That is
 * because, even on 32-bit systems, PHP's RAND_MAX will often be the maximum *signed*
 * value, with only the equivalent of 31 significant bits. Producing two 16-bit random
 * numbers to make up a 32-bit one is less efficient, but guarantees that all 32 bits
 * are random.
 *
 * The algorithm for version 4 UUIDs (ie. those based on random number generators)
 * states that all 128 bits separated into the various fields (32 bits, 16 bits, 16 bits,
 * 8 bits and 8 bits, 48 bits) should be random, except : (a) the version number should
 * be the last 4 bits in the 3rd field, and (b) bits 6 and 7 of the 4th field should
 * be 01. We try to conform to that definition as efficiently as possible, generating
 * smaller values where possible, and minimizing the number of base conversions.
 *
 * Code from http://www.php.net/manual/en/function.uniqid.php#65879
 *
 * @copyright   Copyright (c) CFD Labs, 2006. This function may be used freely for
 *              any purpose ; it is distributed without any form of warranty whatsoever.
 * @author      David Holmes <dholmes@cfdsoftware.net>
 *
 * @return  string  A UUID, made up of 32 hex digits and 4 hyphens.
 */

function uuid() {

    // The field names refer to RFC 4122 section 4.1.2

    return sprintf('%04x%04x-%04x-%03x4-%04x-%04x%04x%04x',
        mt_rand(0, 65535), mt_rand(0, 65535), // 32 bits for "time_low"
        mt_rand(0, 65535), // 16 bits for "time_mid"
        mt_rand(0, 4095),  // 12 bits before the 0100 of (version) 4 for "time_hi_and_version"
        bindec(substr_replace(sprintf('%016b', mt_rand(0, 65535)), '01', 6, 2)),
            // 8 bits, the last two of which (positions 6 and 7) are 01, for "clk_seq_hi_res"
            // (hence, the 2nd hex digit after the 3rd hyphen can only be 1, 5, 9 or d)
            // 8 bits for "clk_seq_low"
        mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535) // 48 bits for "node"
    );
}
