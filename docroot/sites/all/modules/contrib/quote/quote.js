(function ($) {

Drupal.behaviors.quote = {
  attach: function() {
    var level = Drupal.settings.quote_nest - 1;
    if (level >= 0) {
      var top = $('blockquote.quote-nest-1');
      $('blockquote.quote-msg:eq(' + level + ')', top)
      .hide()
      .after('<div class="quote-snip">' + Drupal.t('<a href="#">[snip]</a>') + '</div>')
      .next('.quote-snip')
      .children('a')
      .click(function(e) {
        $(this).parent().siblings('.quote-msg').toggle();
        e.preventDefault();
      });
    }
  }
};

Drupal.behaviors.quoteHighlighted = {
  attach: function(context, settings) {
    $('a.quote-link:not(.quote-processed)', context)
    .addClass('quote-processed')
    .click(function(e) {
      var quoteSettings = settings.quote[this.id];
      var textarea = quoteSettings.textarea;
      // Don't use this behavior if there isn't an edit-body id.
      if (!textarea || $(textarea).length == 0) {
        return true;
      }
      e.preventDefault();
      var selected = getSelectedText();
      var quoted = (selected.length && selected.length > 0) ? selected : jQuery.trim(quoteSettings.body);
      var text = '[quote=' + quoteSettings.author + ']' + quoted + '[/quote]\n';
      insertAtCursor($(textarea).get(0), text);
    });

    /**
     * Insert a piece of text at the cursor in a given textarea.
     *
     * @param object
     *   A javascript (not jquery) object.
     * @param text
     *   The text to insert.
     */
    function insertAtCursor(object, text) {
      // support for the Wysiwyg module
      if (Drupal.wysiwyg && Drupal.wysiwyg.instances[object.id] && Drupal.wysiwyg.instances[object.id].insert && Drupal.wysiwyg.instances[object.id].field == object.id) {
        Drupal.wysiwyg.instances[object.id].insert(text);
      }
      // IE support
      else if (document.selection) {
        object.focus();
        var sel = document.selection.createRange();
        sel.text = text;
        object.focus();
      }
      else if (object.selectionStart || object.selectionStart == '0') {
        var startPos = object.selectionStart;
        var endPos = object.selectionEnd;
        var scrollTop = object.scrollTop;
        object.value = object.value.substring(0, startPos)+text+object.value.substring(endPos,object.value.length);
        object.focus();
        object.selectionStart = startPos + text.length;
        object.selectionEnd = startPos + text.length;
        object.scrollTop = scrollTop;
      }
      else {
        object.value += text;
        object.focus();
      }
    };

    /**
     * Get user highlighted/selected text on page.
     */
    function getSelectedText() {
      if (document.getSelection) {
        return document.getSelection().toString();
      }
      else if (window.getSelection) {
        return window.getSelection().toString();
      }
      else if (document.selection) {
        return document.selection.createRange().text;
      }

      return '';
    }
  }
};

}(jQuery));
