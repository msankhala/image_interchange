<?php
/**
 * @file
 * Describes plugin for the Image Interchange content type.
 * http://foundation.zurb.com/sites/docs/v/5.5.3/components/interchange.html
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Image Interchange'),
  'description' => t('Custom pane that displays images using interchange.'),
  'single' => TRUE,
  'defaults' => array(),
  'edit form' => 'fcl_tm_custom_panels_image_interchange_content_type_edit_form',
  'render callback' => 'fcl_tm_custom_panels_image_interchange_content_type_render',
  'category' => t('TM Custom Panes'),
  'all contexts' => TRUE,
  'defaults' => array(),
);


/**
 * Run-time rendering of the body of the block.
 */
function fcl_tm_custom_panels_image_interchange_content_type_render($subtype, $conf, $args, $contexts) {
  $mobile_image_path = '';
  $tablet_desktop_image = '';
  $tablet_desktop_image_path = '';
  $mobile_image_path = '';

  if ($conf['image_source'] === 'field') {
    $mobile_image_path = ctools_context_keyword_substitute($conf['mobile_field'], array(), $contexts);
    $tablet_desktop_image_path = ctools_context_keyword_substitute($conf['tablet_desktop_field'], array(), $contexts);
  }
  else {
    if (!empty($conf['mobile_image'])) {
      $mobile_image = file_load($conf['mobile_image']);
      $mobile_image_path = file_create_url($mobile_image->uri);
    }

    if (!empty($conf['tablet_desktop_image'])) {
      $tablet_desktop_image = file_load($conf['tablet_desktop_image']);
      $tablet_desktop_image_path = file_create_url($tablet_desktop_image->uri);
    }
  }

  $interchange = fcl_tm_custom_panels_build_interchange_attribute($tablet_desktop_image_path, $mobile_image_path);

  // This is only needed if Equalizer is to be used.
  if ($conf['image_overlay']) {
    drupal_add_js(drupal_get_path('module', 'fcl_tm_custom_panels') . '/plugins/content_types/image_interchange/js/tm.tools-imageinterchange.js');
  }

  $content = array(
    '#theme' => 'image_interchange_output',
    '#interchange' => $interchange,
    '#overlay' => $conf['image_overlay'],
    '#nobanner' => (!empty($interchange) ? '' : 'no-banner'),
  );

  $block = new stdClass();
  $block->content = drupal_render($content);

  return $block;
}

/**
 * Generates block edit form.
 */
function fcl_tm_custom_panels_image_interchange_content_type_edit_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'fcl_tm_custom_panels', 'plugins/content_types/image_interchange/image_interchange');
  $form['#attributes']['enctype'] = array('multipart/form-data');
  $conf = $form_state['conf'];

  $source_options = array(
    'field' => t('Field'),
    'destination_banner' => t('Destination banner'),
    'upload' => t('Upload'),
  );

  $form['image_source'] = array(
    '#type' => 'select',
    '#title' => t('Image Source'),
    '#description' => t('Choose whether to use images from content fields or to upload images.'),
    '#options' => $source_options,
    '#default_value' => isset($conf['image_source']) ? $conf['image_source'] : current(array_keys($source_options)),
  );

  $form['mobile_image'] = array(
    '#type' => 'managed_file',
    '#title' => t('Mobile Image'),
    '#description' => t('Upload an image that will be used for mobile resolutions.'),
    '#upload_location' => 'public://',
    '#default_value' => isset($conf['mobile_image']) ? $conf['mobile_image'] : 0,
    '#states' => array(
      'visible' => array(
        ':input[name="image_source"]' => array('value' => 'upload'),
      ),
    ),
  );

  $form['tablet_desktop_image'] = array(
    '#type' => 'managed_file',
    '#title' => t('Tablet and Desktop Image'),
    '#description' => t('Upload an image that will be used for tablet and desktop resolutions.'),
    '#upload_location' => 'public://',
    '#default_value' => isset($conf['tablet_desktop_image']) ? $conf['tablet_desktop_image'] : 0,
    '#states' => array(
      'visible' => array(
        ':input[name="image_source"]' => array('value' => 'upload'),
      ),
    ),
  );

  $form['mobile_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Mobile Image Field'),
    '#description' => t('Supply the image field name or a token (e.g. %node:field_generic_banner_mobile) for use mobile resolutions.'),
    '#default_value' => isset($conf['mobile_field']) ? $conf['mobile_field'] : '',
    '#states' => array(
      'visible' => array(
        ':input[name="image_source"]' => array('value' => 'field'),
      ),
    ),
  );

  $form['tablet_desktop_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Tablet and Desktop Image Field'),
    '#description' => t('Supply the image field name or token (e.g. %node:field_generic_banner) for use on tablet and desktop resolutions.'),
    '#default_value' => isset($conf['tablet_desktop_field']) ? $conf['tablet_desktop_field'] : '',
    '#states' => array(
      'visible' => array(
        ':input[name="image_source"]' => array('value' => 'field'),
      ),
    ),
  );

  $form['image_overlay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add overlay across image.'),
    '#default_value' => isset($conf['image_overlay']) ? $conf['image_overlay'] : FALSE,
  );

  return $form;
}


/**
 * Submit handler for edit form.
 */
function fcl_tm_custom_panels_image_interchange_content_type_edit_form_submit($form, &$form_state) {
  foreach (element_children($form) as $key) {
    if (isset($form_state['values'][$key])) {
      $value = $form_state['values'][$key];

      switch ($key) {
        case 'mobile_image':
        case 'tablet_desktop_image':
          $image_default = (!empty($form[$key]['#default_value']) ? $form[$key]['#default_value'] : 0);

          if ($value && $image_default && ($image_default !== $value)) {
            // Replace the image when the new image is different to the old one.
            fcl_tm_custom_panels_remove_image($image_default);
            fcl_tm_custom_panels_add_image($value);
          }
          elseif ($value && !$image_default) {
            // Add an image when there is none currently.
            fcl_tm_custom_panels_add_image($value);
          }
      }

      $form_state['conf'][$key] = $value;
    }
    elseif (($key === 'mobile_image' || $key === 'tablet_desktop_image') &&
      !empty($form[$key]['#default_value'])) {
      // Remove the current image if there is no managed fid supplied.
      fcl_tm_custom_panels_remove_image($form[$key]['#default_value']);
      $form_state['conf'][$key] = 0;
    }
  }
}


/**
 * Remove managed file.
 *
 * @param string $managed_file
 *   The file id of the image.
 */
function fcl_tm_custom_panels_remove_image($managed_file) {
  $file = $managed_file ? file_load($managed_file) : FALSE;

  if ($file) {
    file_usage_delete($file, 'fcl_tm_custom_panels', 'image_interchange', $file->id);
    file_delete($file);
  }
}


/**
 * Add managed file.
 *
 * @param string $managed_file
 *   The file id of the image.
 */
function fcl_tm_custom_panels_add_image($managed_file) {
  // If our file is already in use, then we don't need to re-do this and
  // increase the count.
  $count = db_query('SELECT count FROM {file_usage} WHERE fid=:fid', array('fid' => $managed_file))->fetchField();

  if (empty($count)) {
    $file = file_load($managed_file);
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);
    file_usage_add($file, 'fcl_tm_custom_panels', 'image_interchange', $managed_file);
  }
}


/**
 * Build interchange data attribute.
 *
 * @param string $large_path
 *   The path to the image to be loaded on desktop and tablet via interchange.
 *
 * @param string $small_path
 *   The optional path to the image to be loaded on mobile via interchange.
 */
function fcl_tm_custom_panels_build_interchange_attribute($large_path, $small_path) {
  $config = '';

  if (!empty($small_path) && !empty($large_path)) {
    $config = '[' . $small_path . ', (default)], [' . $large_path . ', (medium)]';
  }
  elseif (!empty($large_path)) {
    $config = '[' . $large_path . ', (default)]';
  }

  return $config;
}
