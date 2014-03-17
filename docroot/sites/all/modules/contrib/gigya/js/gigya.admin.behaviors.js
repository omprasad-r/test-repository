/**
 * @file
 * Drupal behaviors for admin pages.
 */

(function ($) {
  /**
   * @todo Undocumented Code!
   */
  Drupal.behaviors.gigyaFiledSettingHideShow = {
    attach: function (context, settings) {
      $('.reactions-override').each( function () {
        if (!$(this).is(':checked')) {
          $(this).parent().next('.gigya-reaction-field-settings').hide();
        }
      });
      $('.reactions-override').once().click(function() {
        $(this).parent().next('.gigya-reaction-field-settings').slideToggle();
      });
      $('.sharebar-override').each( function () {
        if (!$(this).is(':checked')) {
          $(this).parent().next('.gigya-sharebar-field-settings').hide();
        }
      });
      $('.sharebar-override').once().click(function() {
        $(this).parent().next('.gigya-sharebar-field-settings').slideToggle();
      });
      }
  };
    Drupal.behaviors.gigyaAdmin = {
      attach: function (context, settings) {
          Drupal.gigya.checkAdminRadio();
        $('#edit-gigya-login-mode input:radio').once().change( function (e) {
          Drupal.gigya.checkAdminRadio();
        });
      }
    };
    Drupal.gigya.checkAdminRadio = function () {
      $('#edit-gigya-login-mode input:radio').each( function () {
        var val = $(this).val();
        if ((val === 'drupal_and_gigya') || (val === 'gigya') ) {
          if  ($(this).is(':checked') === true){
            $(this).siblings('.description').find('.warnning').removeClass('hidden');
            $('#edit-gigya-social-login, #edit-gigya-login-advanced, #edit-gigya-connect-advanced').show();
            if (val === 'drupal_and_gigya') {
              $(this).parent().next().find('.warnning').addClass('hidden');
            }
            return false;
          }
          else {
            $('#edit-gigya-social-login, #edit-gigya-login-advanced, #edit-gigya-connect-advanced').hide();
            $(this).siblings('.description').find('.warnning').addClass('hidden');
          }
          }
      })
    }
})(jQuery);
