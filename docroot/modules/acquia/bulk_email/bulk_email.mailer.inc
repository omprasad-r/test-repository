<?php
/**
 * @file
 * Bulk mail third party integration
 */

class BulkMailSystem extends DefaultMailSystem implements MailSystemInterface {
  /**
   * Send a message composed by drupal_mail().
   *
   * @param $message
   *   Message array with at least the following elements:
   *   - id: A unique identifier of the e-mail type. Examples: 'contact_user_copy',
   *     'user_password_reset'.
   *   - to: The mail address or addresses where the message will be sent to.
   *     The formatting of this string must comply with RFC 2822. Some examples:
   *     - user@example.com
   *     - user@example.com, anotheruser@example.com
   *     - User <user@example.com>
   *     - User <user@example.com>, Another User <anotheruser@example.com>
   *    - subject: Subject of the e-mail to be sent. This must not contain any
   *      newline characters, or the mail may not be sent properly.
   *    - body: Message to be sent. Accepts both CRLF and LF line-endings.
   *      E-mail bodies must be wrapped. You can use drupal_wrap_mail() for
   *      smart plain text wrapping.
   *    - headers: Associative array containing all additional mail headers not
   *      defined by one of the other parameters.  PHP's mail() looks for Cc
   *      and Bcc headers and sends the mail to addresses in these headers too.
   *
   * @return
   *   TRUE if the mail was successfully accepted for delivery, otherwise FALSE.
   */
  public function mail(array $message) {
    // TODO: custom implementation
    // return the true or fault status if the mail was sent..
    return FALSE;
  }
}