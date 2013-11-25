(function ($) {

/**
 * Command to redirect the browser.
 */
Drupal.ajax.prototype.commands.gardens_media_redirect = function(ajax, response, status) {
  window.location = response.url;
};

/**
 * Media upload page behaviour
 */
Drupal.behaviors.gardens_media = {
  attach: function (context, settings) {
    $('#gardens-media-field-bundle-wrapper').hide();
    $('.gardens-media-embed-code-button').show();
    // Set the default type
    if(typeof Drupal.settings.gardensMedia != 'undefined') {
      // Populate our settings variables
      defaultType = Drupal.settings.gardensMedia.defaultType;
      types = Drupal.settings.gardensMedia.types;
      radios = '';
      // Generate a radio button for each link
      $('.media-upload-ajax-link').each(function(){
        type = $(this).attr('type');
        console.log(types);
        checked = (defaultType == type) ? 'checked="yes"': '';
        radios += '<input type="radio" name="node-type" ' + checked + ' value="' + type + '" class="' + type + '-radio-button" /> ' + types[type] + ' ';
      });

      // Hide our links
      $('.media-upload-ajax-link').hide();
      // Append the radio buttons to the selector
      $('#gardens-media-node-selector-radios').html(radios);

      // If a radio button is selected click the corresponding link
      $('input[name=node-type]:radio').change(function(){
          $('.messages.error').hide();
          type = $(this).val();
          $('#' + type + '-ajax-link').trigger('click');
      });
    }
  }
};
  
})(jQuery);