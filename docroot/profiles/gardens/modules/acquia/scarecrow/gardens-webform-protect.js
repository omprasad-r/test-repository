// $Id$
(function ($) {

Drupal.behaviors.gardensWebformProtect = {
  attach: function (context, settings) {
    $('#edit-form-settings', context).once('gardens-webform-protect', function () {
      // Define selectors for the spam protection checkbox and its hidden
      // counterpart.
      var $spam_protection = $('.form-item-spam-protection', $(this));
      var $spam_protection_checkbox = $('#edit-spam-protection', $spam_protection);
      var $spam_protection_description = $('.description', $spam_protection);
      var $spam_protection_force_enabled = $('input[name="spam_protection_force_enabled"]', $(this));

      // Define selectors for the anonymous user checkbox.
      var $anonymous_user = $('.form-item-roles-1', $(this));
      var $anonymous_user_checkbox = $('#edit-roles-1', $anonymous_user);
      var $anonymous_user_description = $('.description', $anonymous_user);

      var spamProtectionChanged = function () {
        // When spam protection is turned on, allow the anonymous user checkbox
        // to be set to whatever the administrator wants.      
        if ($(this).is(':checked')) {
          $anonymous_user_checkbox.removeAttr('disabled');
          $anonymous_user_description.html('');
        }
        // When spam protection is turned off, the anonymous user checkbox
        // needs to be unchecked and disabled.
        else {
          $anonymous_user_checkbox.removeAttr('checked');
          $anonymous_user_checkbox.attr('disabled', 'disabled');
          $anonymous_user_description.html(Drupal.t('You must turn spam protection on to allow this webform to be accessed by anonymous users'));
        }
      };

      var anonymousUserChanged = function () {
        // When anonymous users access to the webform is turned on, the spam
        // protection checkbox needs to be checked and disabled.
        if ($(this).is(':checked')) {
          $spam_protection_checkbox.attr('checked', 'checked');
          $spam_protection_checkbox.attr('disabled', 'disabled');
          $spam_protection_description.html(Drupal.t("To disable spam protection, first remove the <em>anonymous user</em> role's access to this webform."));
          // Also set the hidden field indicating that the checkbox should be
          // treated as checked on the server side (even though the browser
          // won't send a value for the checkbox itself, since it's disabled).
          $spam_protection_force_enabled.val(1);
        }
        // When anonymous user access to the webform is turned off, allow the
        // spam protection checkbox to be set to whatever the administrator
        // wants.
        else {
          $spam_protection_checkbox.removeAttr('disabled');
          $spam_protection_description.html('');
          // Reset the hidden field.
          $spam_protection_force_enabled.val(0);
        }
      };

      // Trigger the above functions when the checkboxes are toggled.
      $spam_protection_checkbox.bind('change.gardensWebformProtect', spamProtectionChanged);
      $anonymous_user_checkbox.bind('change.gardensWebformProtect', anonymousUserChanged);

      // Trigger them once at the beginning to initialize things correctly.
      $spam_protection_checkbox.triggerHandler('change.gardensWebformProtect');
      $anonymous_user_checkbox.triggerHandler('change.gardensWebformProtect');
    });
  }
};

})(jQuery);
