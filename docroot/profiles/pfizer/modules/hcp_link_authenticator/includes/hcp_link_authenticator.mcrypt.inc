<?php
/**
 * @file
 * Facilitates encryption and decryption via mcrypt (blowfish).
 */

/**
 * Defines the fixed IV for the CBC-mode encryption.
 */
define('HCP_LINK_IV', 'y56N1Qes');

/**
 * Encrypts the given string using blowfish.
 *
 * @param $string
 *   Any text or group of characters.
 * @return
 *   An encrypted string.
 */
function _hcp_link_authenticator_encrypt($string) {
  $mcrypt = mcrypt_encrypt(MCRYPT_BLOWFISH, md5(variable_get('hcp_link_passphrase_central_link')), $string, MCRYPT_MODE_CBC, HCP_LINK_IV);

  return rtrim(strtr(base64_encode($mcrypt), '+/', '-_'), '=');
}

/**
 * Decrypts the given string using blowfish.
 *
 * @param $string
 *   Blowfish-encrypted ciphertext.
 * @return
 *   A decrypted string.
 */
function _hcp_link_authenticator_decrypt($string) {
  $data = strtr($string, '-_', '+/');
  if (strlen($data) % 4 > 0) {
    $pad_length = strlen($string) + (4 - (strlen($string) % 4));
    $data = str_pad($data, $pad_length, '=', STR_PAD_RIGHT);
  }

  $mcrypt = base64_decode($data);

  return mcrypt_decrypt(MCRYPT_BLOWFISH, md5(variable_get('hcp_link_passphrase_central_link')), $mcrypt, MCRYPT_MODE_CBC, HCP_LINK_IV);
}

