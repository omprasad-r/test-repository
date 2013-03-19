<?php

class GardensWordPressBlogEntry extends WordPressBlogEntry {
  /**
   * Rewrite [caption] tags into HTML representing a caption. [caption] itself
   * ($matches[1]) will become an opening <div>, the content within the tag
   * ($matches[2]) will be passed through unchanged, and the closing [/caption]
   * ($matches[3]) will become a <p> containing the caption followed by a closing </div>.
   *
   * @param array $matches
   */
  protected function replaceCaptions(array $matches) {
    return gardens_migrate_caption_callback($matches);
  }
}

class GardensWordPressPage extends WordPressPage {
  /**
   * Rewrite [caption] tags into HTML representing a caption. [caption] itself
   * ($matches[1]) will become an opening <div>, the content within the tag
   * ($matches[2]) will be passed through unchanged, and the closing [/caption]
   * ($matches[3]) will become a <p> containing the caption followed by a closing </div>.
   *
   * @param array $matches
   */
  protected function replaceCaptions(array $matches) {
    return gardens_migrate_caption_callback($matches);
  }
}

/**
 * Rewrite [caption] tags into HTML representing a caption. [caption] itself
 * ($matches[1]) will become an opening <div>, the content within the tag
 * ($matches[2]) will be passed through unchanged, and the closing [/caption]
 * ($matches[3]) will become a <p> containing the caption followed by a closing </div>.
 *
 * @param array $matches
 */
function gardens_migrate_caption_callback(array $matches) {
  $caption_open = $matches[1];
  $content = $matches[2];
  $caption_close = $matches[3];

  preg_match('|width="(.*?)"|i', $caption_open, $matches);
  $width = (int)$matches[1] + 10;
  $style = "width: {$width}px;";

  preg_match('|align="(.*?)"|i', $caption_open, $matches);
  $align = $matches[1];

  preg_match('|caption="(.*?)"|i', $caption_open, $matches);
  $caption = $matches[1];

  $result = '<div class="' . $align . '"style="' . $style . '">';
  $result .= $content;
  $result .= "<p>$caption</p></div>";

  return $result;
}
