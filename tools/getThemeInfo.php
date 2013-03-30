#!/usr/bin/php -q
<?php

if (count($argv) != 2) {
  printf("Usage: %s <theme path>\n", $argv[0]);
  exit(1);
}
else {
  print getThemeSummary($argv[1]);
}

function getThemeSummary($path) {
  $path_info = pathinfo($path);
  $theme_name = $path_info['basename'];
  $info_file = file_get_contents("$path/$theme_name.info");
  $info = parse_info_format($info_file);

  $version = $info['version'];
  $base_theme = $info['base_theme'];
  if (empty($base_theme)) {
    $base_theme = get_base_theme_name_from_screenshot($path, $theme_name);
  }
  $advancedLines = getAdvancedCSSLineCount($path);
  return sprintf("%s,%s,%d,%s\n", $base_theme, $version, $advancedLines, $path);
}

function getAdvancedCSSLineCount($path) {
  $advancedInfo = file_get_contents("$path/advanced.css");
  $advancedLines = explode("\n", $advancedInfo);
  return count($advancedLines);
}

/**
 * In the original release of Gardens, we did not have the base_theme property
 * in the .info file.  This is the only way we know the origin of the theme
 * after it has been saved.  This simple scheme relies on the fact that
 * originally we did not replace the thumbnail images for the theme
 * (screenshot_lg.png), so it is possible to look at the thumbnail for these
 * older themes and determine the base theme from there.  Eventually this will
 * be dead code because all of the themes should be converted over time.
 *
 * @param $theme
 *   Either the theme object or the theme name.
 */
function get_base_theme_name_from_screenshot($path, $theme) {
  $map = array(
    '92bdf6d0f4d916981ffd03e7d819faa7' => 'broadway',
    '87ce6808dd821638aa4e10076290af15' => 'builderbase',
    'd71e2135c1f58c77bda84e318f0def7e' => 'campaign',
    '0f7cfc45ea93e61090c719fd498d07d1' => 'kenwood',
    '5d6e1abd629a5774d851d28c0fa3e19c' => 'sonoma',
    '1818a2557c35553c444d7c72c8ea740b' => 'sparks',
  );
  if (is_string($theme)) {
    // Passed in the theme name.
    $name = $theme;
  }
  else {
    $path = $theme->getPath();
    $name = $theme->getName();
  }
  if (empty($path)) {
    $path = drupal_get_path('theme', $name);
  }
  $screenshot_path = $path . '/screenshot_lg.png';
  $contents = file_get_contents($screenshot_path);
  $md5 = md5($contents);
  return $map[$md5];
}

// Totally ripped from drupal_parse_info_format.
function parse_info_format($data) {
  $info = array();
  $constants = get_defined_constants();

  if (preg_match_all('
    @^\s*                           # Start at the beginning of a line, ignoring leading whitespace
    ((?:
      [^=;\[\]]|                    # Key names cannot contain equal signs, semi-colons or square brackets,
      \[[^\[\]]*\]                  # unless they are balanced and not nested
    )+?)
    \s*=\s*                         # Key/value pairs are separated by equal signs (ignoring white-space)
    (?:
      ("(?:[^"]|(?<=\\\\)")*")|     # Double-quoted string, which may contain slash-escaped quotes/slashes
      (\'(?:[^\']|(?<=\\\\)\')*\')| # Single-quoted string, which may contain slash-escaped quotes/slashes
      ([^\r\n]*?)                   # Non-quoted string
    )\s*$                           # Stop at the next end of a line, ignoring trailing whitespace
    @msx', $data, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      // Fetch the key and value string
      $i = 0;
      foreach (array('key', 'value1', 'value2', 'value3') as $var) {
        $$var = isset($match[++$i]) ? $match[$i] : '';
      }
      $value = stripslashes(substr($value1, 1, -1)) . stripslashes(substr($value2, 1, -1)) . $value3;

      // Parse array syntax
      $keys = preg_split('/\]?\[/', rtrim($key, ']'));
      $last = array_pop($keys);
      $parent = &$info;

      // Create nested arrays
      foreach ($keys as $key) {
        if ($key == '') {
          $key = count($parent);
        }
        if (!isset($parent[$key]) || !is_array($parent[$key])) {
          $parent[$key] = array();
        }
        $parent = &$parent[$key];
      }

      // Handle PHP constants.
      if (isset($constants[$value])) {
        $value = $constants[$value];
      }

      // Insert actual value
      if ($last == '') {
        $last = count($parent);
      }
      $parent[$last] = $value;
    }
  }

  return $info;
}
