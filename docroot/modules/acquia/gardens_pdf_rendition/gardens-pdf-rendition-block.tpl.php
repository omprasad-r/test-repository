<?php if ($in_progress): ?>
<p>
  <?php print t('New PDF snapshot is in queue for generation...'); ?>
</p>
<?php elseif ($data_available): ?>
<p>
  <strong><?php print t('Download most recent'); ?></strong>
</p>
<div class="clearfix gardens-pdf-rendition-file-container">
  <img class="gardens-pdf-rendition-thumbnail"
       src="<?php print $thumbnail_image; ?>"
       alt="<?php t('PDF Thumbnail') ?>"/>
  <a class="gardens-pdf-rendition-link"
     href="<?php print $pdf_link; ?>"><?php print $pdf_generation_timestamp; ?></a>
  <span class="gardens-pdf-rendition-filesize">(<?php print $pdf_size; ?>) PDF</span>
</div>
<?php else: ?>
<p>
  <?php print t('No PDF snapshot of the this page has been made yet.'); ?>
</p>
<?php endif; ?>
<?php if ($link_to_regeneration && !$in_progress): ?>
<p class="gardens-pdf-rendition-description">
  <?php print t('Capture a new copy of this page in PDF format and have it sent to your email address.'); ?>
</p>
<p>
  <a class="gardens-pdf-rendition-generate"
     href="<?php print $link_to_regeneration; ?>"><?php print t('Generate new snapshot'); ?></a>
</p>
<?php endif; ?>
