<?php
// $Id: toolbar.tpl.php,v 1.11 2010/05/23 18:23:32 dries Exp $

/**
 * @file
 * Default template for admin toolbar.
 *
 * Available variables:
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default value has the following:
 *   - toolbar: The current template type, i.e., "theming hook".
 * - $toolbar['toolbar_user']: User account / logout links.
 * - $toolbar['toolbar_menu']: Top level management menu links.
 * - $toolbar['toolbar_drawer']: A place for extended toolbar content.
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * @see template_preprocess()
 * @see template_preprocess_toolbar()
 */
?>
<div id="toolbar" class="<?php print $classes; ?> clearfix">
  <div class="toolbar-menu clearfix">
    <div class="toolbar-left">
      <?php print render($toolbar['toolbar_left']); ?>
    </div>
    <div class="toolbar-right">
      <?php print render($toolbar['toolbar_right']); ?>
    </div>
  </div>
  
  <?php print render($toolbar['toolbar_toggle']); ?>

  <div class="<?php echo $toolbar['toolbar_drawer']['toolbar_drawer_classes']; ?>">
    <?php print render($toolbar['toolbar_drawer']); ?>
  </div>
  
  <?php if (isset($toolbar['tooltips'])): ?>
  <div class="toolbar-tooltips"></div>
  <?php endif; ?>
  
</div>
