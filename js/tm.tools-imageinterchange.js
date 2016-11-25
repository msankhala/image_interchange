/**
 * @file
 * This file contains code to reflow Equalizer when Interchange kicks in.
 */

+function($, Drupal, document, window, undefined) {
  'use strict';

  Drupal.behaviors.tm_tools_imageinterchange = {
    initialised: false,

    attach: function (context, settings) {
      if (!this.initialised) {
        var $document = $(document);

        $document.on('replace', '.image-interchange img', function() {
          $document.foundation('equalizer', 'reflow');
        });

        this.initialised = true;
      }
    },
  };
} (jQuery, Drupal, this.document, this);
