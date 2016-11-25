<?php 
/**
 * @file
 * This file is to contain markup for the Image Interchange pane.
 */
?>

<div class="image-interchange <?php print $nobanner; ?>">
  <?php if ($overlay): ?>
    <div class="overlay__black-stripes"></div>
  <?php endif; ?>
  <?php if (!empty($interchange)): ?>
    <img data-interchange="<?php print $interchange; ?>">
  <?php endif; ?>
</div>
