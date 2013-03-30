(function ($) {

// Makes sure that strings support the trim() method from ECMAScript 5.
if (typeof String.prototype.trim != 'function') { // detect native implementation
  String.prototype.trim = function () {
    return this.replace(/^\s+/, '').replace(/\s+$/, '');
  };
}

Drupal.behaviors.gardensMiscComment = {
  attach: function (context) {
    // Overrides the summary handler from comment-node-form.js.
    $('fieldset.comment-node-type-settings-form', context).drupalSetSummary(function (context) {
      var vals = [];

      // Default comment setting.
      vals.push($("label", $(".form-item-comment input[type=radio][name=comment]:checked", context).parent()).text().trim());

      // Threading.
      var threading = $(".form-item-comment-default-mode input:checked", context).next('label').text();
      if (threading) {
        vals.push(threading);
      }

      // Comments per page.
      var number = $(".form-item-comment-default-per-page input", context).val();
      vals.push(Drupal.t('@number comments per page', {'@number': number}));

      return Drupal.checkPlain(vals.join(', '));
    });
  }
};

})(jQuery);
