
/**
 * LoginToboggan needs its pre-auth role to not be explicitly tied to
 * the auth role.
 */
(function ($) {

/**
 * Shows checked and disabled checkboxes for inherited permissions.
 */
Drupal.behaviors.LoginTobogganPermissions = {
  attach: function (context, settings) {
    // Revert changes made by modules/user/user.permissions.js
    $('table#permissions', context).once('tobogganPermissions', function () {
      $('input[type=checkbox]', this).filter('.rid-' + settings.LoginToboggan.preAuthID).removeClass('real-checkbox').each(function () {
        $(this).next().filter('.dummy-checkbox').remove();
        $('input.rid-' + settings.LoginToboggan.preAuthID).each(function () {
          this.style.display = '';
        });
      });
    });
  },
};

})(jQuery);
