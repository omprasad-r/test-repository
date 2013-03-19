<?php
/**
 * @file
 * Theme implementation to display the MASt login flow.
 *
 * Strips out all regions but $page['content'].
 */
?>
<div class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php print $messages; ?>

  <div id="content">
    <?php print render($title_prefix); ?>
    <?php if ($title): ?>
      <h1 class="title" id="page-title"><?php print $title; ?></h1>
    <?php endif; ?>
    <?php print render($title_suffix); ?>

    <?php print render($page['content']); ?>
  </div>
</div>

