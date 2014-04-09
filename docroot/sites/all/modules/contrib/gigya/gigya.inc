<?php

/**
 * @file
 * @todo file description.
 *
 * Development of this module is sponsered by Gigya, Inc.
 */

/**
 * Makes Gigya token replacements.
 *
 * Callback for array_walk.
 *
 * @param string $value
 *   A refernce to the string to manipulate.
 * @param string $key
 *   The key of the value from the array.
 * @param array $params
 *   - replacements : An associative array containing the Gigya tokens.
 *   - token_types : An keyed array with the token types , the token types could
 *     by 'node' or 'user' or 'comment'.
 *   - object : stdClass could by node or comment or user.
 *   - type : string the object type could by 'node' or 'comment' or 'user'
 *
 * @see gigya_shareui_general_action()
 */

/**
 * Callback for array_walk().
 *
 * Receives form items and assesses whether or not they're input fields.
 * If so, then it tries to set the default value based on the profile mappings.
 *
 * see gigya_form_alter()
 */
function gigya_apply_profile_mapping(&$field, $key) {
  if (is_array($field)) {
    // Check for some key elements. We do this explicitly to make sure the
    // indexes are defined and avoid PHP notices.
    $type = isset($field['#type']) ? $field['#type'] : '';
    $field_name = isset($field['#field_name']) ? $field['#field_name'] : '';
    $theme = isset($field['#theme']) ? $field['#theme'] : '';

    // Is it a standard type to replace?
    if (in_array($type, array(
        'checkbox',
        'checkboxes',
        'date',
        'radio',
        'radios',
        'select',
        'textarea',
        'textfield',
      )
    )
    ) {
      if (isset($field['#default_value'])) {
        $orig = $field['#default_value'];
        $token = token_replace(variable_get('gigya_map_' . $key, ''));
        $field['#default_value'] = $token ? $token : $orig;
      }
    }
    // If it isn't and we have a #field_name, just go for it.
    elseif ($field_name && $type) {
      if (isset($field['#default_value'])) {
        $orig = $field['#default_value'];
        $token = token_replace(variable_get('gigya_map_' . $field['#field_name'], ''), 'gigya');
        $field['#default_value']['value'] = $token ? $token : $orig;
      }
    }
    // If it is a fieldset or multi-value field, drill in.
    elseif ($type == 'fieldset' || $theme == 'content_multiple_values') {
      array_walk($field, 'gigya_apply_profile_mapping');
    }
  }
}

/**
 * Get an avatar picutre url from Gigya by $uid.
 *
 * @param string $uid
 *   The user id to get the avatar for.
 *
 * @return bool
 *   The user thumbnail url if he has one or the photoURL if not.
 */
function gigya_avatar_check($uid) {
  $bio = gigya_get_user_info($uid);
  if ($bio['thumbnailURL']) {
    return $bio['thumbnailURL'];
  }
  elseif ($bio['photoURL']) {
    return $bio['photoURL'];
  }
  else {
    return FALSE;
  }
}

/**
 * Save an user picture from facebook and resize it.
 *
 * @param string $uid
 *   the user id to get the picture for.
 *
 * @return bool|object
 *   A file object if the user has avatar picture and we successfuly save the
 */
function gigya_save_userpic($uid) {
  // Get Facebook user picture's url.
  if ($avatar = gigya_avatar_check($uid)) {
    $result = drupal_http_request($avatar);

    if ($result->code != 200) {
      watchdog('gigya', 'Failed importing user avatar for @uid, code : @code', array(
          '@uid' => $uid,
          '@code' => $result->code,
        )
      );
      return FALSE;
    }

    $filename = 'picture-gigya_' . $uid . '.jpg';
    $dest = file_directory_path();
    if (variable_get('user_picture_path', '')) {
      $dest .= '/' . variable_get('user_picture_path', '');
    }
    $dest .= '/' . $filename;

    $file = file_save_data($result->data, $dest, FILE_EXISTS_REPLACE);
    // Resizing.
    list($width, $height) = sscanf(variable_get('user_picture_dimensions', '85x85'), '%dx%d');
    image_scale_and_crop($file, $dest, $width, $height);
    drupal_set_message(t('User picture saved successfully!'));

    return $file;
  }
  else {
    drupal_set_message(t('Your social network does not have a profile picture'));
    return FALSE;
  }
}

/**
 * Prepare a keyed array of languages.
 *
 * @return array
 *   Keyed array of languages.
 */
function gigya_get_languages() {
  $languages = array(
    'en' => 'English',
    'zh-cn' => 'Chinese',
    'zh-hk' => 'Chinese (Hong Kong)',
    'zh-tw' => 'Chinese (Taiwan)',
    'cs' => 'Czech',
    'da' => 'Danish',
    'nl' => 'Dutch',
    'fi' => 'Finnish',
    'fr' => 'French',
    'de' => 'German',
    'el' => 'Greek',
    'hu' => 'Hungarian',
    'id' => 'Indonesian',
    'it' => 'Italian',
    'ja' => 'Japanese',
    'ko' => 'Korean',
    'ms' => 'Malay',
    'no' => 'Norwegian',
    'pl' => 'Polish',
    'pt' => 'Portuguese',
    'pt-br' => 'Portuguese (Brazil)',
    'ro' => 'Romanian',
    'ru' => 'Russian',
    'es' => 'Spanish',
    'es-mx' => 'Spanish (Mexican)',
    'sv' => 'Swedish',
    'tl' => 'Tagalog (Philippines)',
    'th' => 'Thai',
    'tr' => 'Turkish',
    'uk' => 'Ukrainian',
    'vi' => 'Vietnamese',
  );
  return $languages;
}

/**
 * Gets the list of the Gigya profile fields.
 *
 * @return array
 *   An associative array (machine-name => human-readable display name).
 */
function _gigya_get_profile_fields() {
  return array(
    'nickname' => t('Nickname'),
    'photoURL' => t('Gigya Profile photo URL'),
    'thumbnailURL' => t('Thumbnail profile photo URL'),
    'profileURL' => t('Profile URL'),
    'firstName' => t('First name'),
    'lastName' => t('Last name'),
    'gender' => t('Gender'),
    'age' => t('Age'),
    'birthDay' => t('Birth day'),
    'birthMonth' => t('Birth month'),
    'birthYear' => t('Birth year'),
    'email' => t('Email'),
    'country' => t('Country'),
    'state' => t('State'),
    'city' => t('City'),
    'zip' => t('Zip'),
    'likes' => t('likes'),
    'proxiedEmail' => t('proxiedEmail'),
    'languages' => t('languages'),
    'address' => t('address'),
    'honors' => t('honors'),
    'professionalHeadline' => t('professionalHeadline'),
    'bio' => t('bio'),
    'industry' => t('industry'),
    'specialties' => t('specialties'),
    'religion' => t('religion'),
    'politicalView' => t('politicalView'),
    'interestedIn' => t('interestedIn'),
    'relationshipStatus' => t('relationshipStatus'),
    'hometown' => t('hometown'),
    'username' => t('Facebook Username'),
    'locale' => t('locale'),
    'ltimezone' => t('ltimezone'),
  );
}

/**
 * Callback for array_walk, helper function for trimming.
 *
 * @value string
 *   The string to trim.
 */
function _gigya_trim_value(&$value) {
  $value = trim($value);
}

/**
 * Logs user in to Gigya's service and optionally registers them.
 *
 * @param string $uid
 *   The drupal User ID.
 * @param bool $is_new_user
 *   Tell Gigya if we add a new user.
 * @param mixed $user_info_json
 *   Extra info for the user.
 *   see gigya_user_login()
 *
 * @return bool|null|string
 *   True if the notify login request succeeded or the error message from Gigya.
 */
function gigya_notify_login($uid, $is_new_user = FALSE, $user_info_json = NULL) {
  $apikey = variable_get('gigya_APIKey', '');
  $secretkey = variable_get('gigya_SecretKey', '');
  $request = new GSRequest($apikey, $secretkey, 'socialize.notifyLogin');
  $request->setParam("siteUID", $uid);
  if (!empty($is_new_user)) {
    $request->setParam('newUser', TRUE);
  }
  if (!empty($user_info_json)) {
    $request->setParam('user_info', $user_info_json);
  }
  $response = $request->send();
  if ($response->getErrorCode() !== 0) {
    // Error.
    watchdog('gigya', 'error in notify login');
    return $response->getErrorMessage();
  }
  try {
    setcookie($response->getString("cookieName"), $response->getString("cookieValue"), 0, $response->getString("cookiePath"), $response->getString("cookieDomain"));
  }
  catch (Exception $e) {
    watchdog('gigya', 'error seting gigya cookie');
    watchdog('gigya', 'error message : @error', array('@error' => $e->getMessage()));
  }

  return TRUE;
}

/**************************************************
 *  Internal Gigya signature verification functions
 **************************************************/

/**
 * Helper function that validates the SDK response.
 *
 * @param mixed $response
 *   The SDK response.
 *
 * @return bool
 *   True if response signature is valid false if not.
 */
function _gigya_validate_sig($response) {
  $secretkey = variable_get('gigya_SecretKey', '');
  $valid = SigUtils::validateUserSignature($response->getString("UID", ""), $response->getString("signatureTimestamp", ""), $secretkey, $response->getString("UIDSignature", ""));
  if ($valid) {
    return TRUE;
  }
  else {
    watchdog('gigya', 'Response signature is not valid.', NULL, WATCHDOG_WARNING);
    return FALSE;
  }
}

/**
 * A helper function that calculate the Gigya signature.
 *
 * @param int $timestamp
 *   The request timestamp.
 * @param mixed $user_id
 *   The Gigya user id.
 *
 * @return string
 *   The calculated signature.
 */
function _gigya_calculate_signature($timestamp, $user_id) {
  $base_str = $timestamp . '_' . $user_id;
  $key = variable_get('gigya_SecretKey', FALSE);
  $sig = SigUtils::calcSignature($base_str, $key);
  return $sig;
}

/**
 * Validates the Gigya session keys.
 *
 * We use the site 'admin' username to find out the status. If it shows the
 * user logged out, thats good, if it returns an error then our keys are
 * most likely bad.
 */
function gigya_validate($apikey, $secretkey, $data_center, $update = FALSE) {
  $orig_validate_state = variable_get('gigya_validate', FALSE);
  $request = new GSRequest($apikey, $secretkey, 'shortenURL');
  $request->setAPIDomain($data_center);
  $request->setParam('url', 'http://gigya.com');
  $response = $request->send();
  $error = $response->getErrorCode();
  if (!$update) {
    if ($error == 0) {
      drupal_set_message(t('Gigya validated properly. This site is authorized to use Gigya services'));
      variable_set('need_validation_msg', TRUE);
      variable_set('gigya_validate', TRUE);
      return TRUE;
    }
    else {
      variable_set('gigya_validate', FALSE);
      return $response;
    }
  }
  if (!$orig_validate_state || variable_get('need_validation_msg', FALSE)) {
    drupal_set_message(t('Gigya validated properly. This site is authorized to use Gigya services'));
    variable_set('need_validation_msg', TRUE);
  }

  variable_set('gigya_validate', TRUE);
  return TRUE;
}

/****************************
 * Gigya API functions
 ****************************/

/**
 * Delete user from Gigya's database.
 *
 * @param int $uid
 *   The drupal user id to delete.
 *
 * internal param int $timestamp The request timestamp.*
 * The request timestamp.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_delete_account($uid) {
  if (empty($uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $uid,
  );

  return _gigya_api('deleteAccount', $params);
}

/**
 * Notifies Gigya that we've completed registering this user.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 * @param int $drupal_uid
 *   internal param int $uid The drupal user id.*
 *   The drupal user id.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_notify_registration($gigya_uid, $drupal_uid) {
  if (empty($gigya_uid) || empty($drupal_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
    'siteUID' => $drupal_uid,
  );

  return _gigya_api('notifyRegistration', $params);
}


/**
 * Sets the Gigya UID to match the Drupal UID.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 * @param int $drupal_uid
 *   internal param int $uid The drupal user id.*
 *   The drupal user id.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_set_uid($gigya_uid, $drupal_uid) {
  if (empty($gigya_uid) || empty($drupal_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
    'siteUID' => $drupal_uid,
  );

  return _gigya_api('setUID', $params);
}

/**
 * Fetches information about user following a given Gigya account.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_get_user_info($gigya_uid) {
  if (empty($gigya_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
  );

  return _gigya_api('getUserInfo', $params);
}

/**
 * Fetches information about the user friends following a given Gigya account.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 * @param array $params
 *   An associative array of params to pass to Gigya.
 *   see http://developers.gigya.com/020_Client_API/020_Methods/socialize.getFriends
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_get_friends_info($gigya_uid, $params) {
  if (empty($gigya_uid)) {
    return FALSE;
  }

  $params += array(
    'uid' => $gigya_uid,
  );

  return _gigya_api('getFriendsInfo', $params);
}

/**
 * Sets the user's status in social networks that support this feature.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 * @param string $status
 *   The status string to set.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_set_status($gigya_uid, $status) {
  if (empty($gigya_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
    'status' => $status,
  );

  return _gigya_api('setStatus', $params);
}

/**
 * Publishes a user action to the newsfeed on all the connected providers.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 * @param array $content
 *   template :  the template.
 *   action : the action.
 *   title : the action title.
 *   path : the path.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_publish_user_action($gigya_uid, $content) {
  if (empty($gigya_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
    'userAction' => _gigya_get_useraction_xml($content),
  );

  return _gigya_api('publishUserAction', $params);
}

/**
 * Log a user out using the Rest API.
 *
 * @param mixed $gigya_uid
 *   The Gigya uid.
 *
 * @return array
 *   The response from Gigya.
 */
function gigya_logout_uid($gigya_uid) {
  if (empty($gigya_uid)) {
    return FALSE;
  }

  $params = array(
    'uid' => $gigya_uid,
  );

  return _gigya_api('logout', $params);
}

/*
 * Handles all calls to the Gigya API.
 */
require_once drupal_get_path('module', 'gigya') . '/GSSDK.php';

/**
 * Helper function that handles Gigya API calls.
 *
 * @param mixed $method
 *   The Gigya API method.
 * @param mixed $params
 *   The method parameters.
 *
 * @param bool $return_code
 *   Should error be returned.
 *
 * @return array
 *   The Gigya response.
 */
function _gigya_api($method, $params, $return_code = FALSE) {
  $apikey = variable_get('gigya_APIKey', '');
  $secretkey = variable_get('gigya_SecretKey', '');
  $request = new GSRequest($apikey, $secretkey, $method);
  $user_info = NULL;
  if (!empty($params)) {
    $user_info = in_array('getUserInfo', $params);
    foreach ($params as $param => $val) {
      $request->setParam($param, $val);
    }
    if (!empty($params['uid'])) {
      $gig_uid = $params['uid'];
    }
  }
  $request->setAPIDomain(variable_get('gigya_data_center', 'us1.gigya.com'));
  $response = $request->send();
  if (variable_get('gigya_debug', FALSE)) {
    $req_dump = print_r($request, TRUE);
    $res_dump = print_r($response, TRUE);
    watchdog('gigya', 'Request sent to gigya <br /><pre>' . $req_dump . '</pre>', NULL, WATCHDOG_DEBUG);
    watchdog('gigya', 'Response from gigya <br /><pre>' . $res_dump . '</pre>', NULL, WATCHDOG_DEBUG);
  }
  if ($code = _gigya_request_failure($response, $user_info, $return_code)) {
    return $code;
  }

  return _gigya_convert_json_to_array($response->getResponseText());
}

/**
 * Internal helper function to deal cleanly with various HTTP response codes.
 *
 * @param mixed $response
 *   the gigya response.
 * @param bool $user_info
 *   tell if the request has the user info parm.
 *
 * @param bool $return_error
 *   Should error be returned.
 *
 * @return bool
 *   true if we have errors false if not.
 */
function _gigya_request_failure($response, $user_info = NULL, $return_error = FALSE) {
  $code = $response->getErrorCode();
  if (variable_get('gigya_debug', 0)) {
    watchdog('gigya', $response->getLog(), NULL, WATCHDOG_DEBUG);
  }
  switch ($code) {
    case '0':
      if ($user_info) {
        if (_gigya_validate_sig($response)) {
          return FALSE;
        }
        else {
          return ($return_error) ? $code : FALSE;
        }
      }
      else {
        return ($return_error) ? $code : FALSE;
      }

    case '403005':
      if (variable_get('gigya_validate', FALSE)) {
        return FALSE;
      }
      else {
        watchdog('gigya', $response->getErrorMessage(), NULL, WATCHDOG_WARNING);
        return (isset($return_error)) ? $code : TRUE;
      }

    default:
      return ($return_error) ? $code : TRUE;
  }
}

/**
 * Convert JSON response to a PHP array.
 *
 * @param string $data
 *   The JSON data.
 *
 * @return array
 *   The converted array from the JSON.
 */
function _gigya_convert_json_to_array($data) {
  $array = json_decode($data, TRUE);
  return $array;
}


/**
 * Checks if user exist by querying the database.
 *
 * @param string $id
 *   email or username.
 *
 * @return bool
 *   True if user exist.
 */
function gigya_dose_user_exist($id) {
  return (bool) db_select('users')->fields('users', array('uid'))->condition('mail', db_like($id), 'LIKE')->range(0, 1)
    ->execute()->fetchField() || (bool) db_select('users')->fields('users', array('uid'))
    ->condition('name', db_like($id), 'LIKE')->range(0, 1)->execute()->fetchField();
}

/**
 * Checks if this email is the primary user email.
 *
 * @param array $user_info
 *   The user info from accounts.getUserInfo api call.
 * @param string $email
 *   The user email.
 *
 * @return bool
 *   True if this email is the primary.
 */
function gigya_is_raas_primary_user($user_info, $email) {
  foreach ($user_info['loginIDs'] as $email_array) {
    if (in_array($email, $email_array)) {
      return TRUE;
    }
  }
  return FALSE;
}
