<?php

/**
 * Implements hook_disallowed_modules_alter().
 */
function pfizer_disallowed_modules_alter(&$modules) {
  $modules = array_diff($modules, array(
    'gardens_pdf_rendition',
    'gardens_pdf_rendition_sitemap',
    'gardens_site_variables',
    'node_export',
    'uuid',
    'janrain_client',
    'janrain_login',
  ));
}

/**
 * Implements hook_init().
 */
function pfizer_init() {
  // Disable the default janrain_login JS to use custom JS for Pfizer.
  $GLOBALS['conf']['janrain_login_add_default_js'] = FALSE;

  // Set the S3 bucket name for PDF rendition uploads (different on prod/staging)
  if (!empty($_ENV['AH_SITE_ENVIRONMENT']) && $_ENV['AH_SITE_ENVIRONMENT'] === 'prod') {
    $GLOBALS['conf']['gardens_pdf_rendition_s3_bucket'] = 'pfizerbucket1';
  }
  else {
    $GLOBALS['conf']['gardens_pdf_rendition_s3_bucket'] = 'gsteamer.site-archives';
  }

  // Disable the iframe-buster script
  $GLOBALS['conf']['gardens_misc_iframe_buster'] = FALSE;

  // Per Pfizer security review, set additional (no-store) cache inhibition headers if HTTPS.
  if (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') {
    drupal_add_http_header('Cache-Control', 'no-cache, no-store, must-revalidate, post-check=0, pre-check=0');
  }
}

/**
 * Implements hook_securepages_goto_alter().
 *
 * @param $url
 */
function pfizer_securepages_goto_alter(&$url) {
  global $user;
  if ($user->uid > 0) {
    $timestamp = REQUEST_TIME;
    // We can't tell for sure if the user has a valid session on the domain for the
    // other protocol, so we have to set this in any case, which adds a small
    // overhead whenever switching protocols.  Do not add the token for any user
    // with elevated privileges.
    if (pfizer_user_basic_role($user)) {
      // We can't generate the hash using $account->login for the HCP user, as
      // a second user logging in with that account via a HCP link could invalidate
      // the first user's protocol switch login.
      $hash = pfizer_is_hcp_user($user) ? user_pass_rehash($user->pass, $timestamp, $timestamp) : user_pass_rehash($user->pass, $timestamp, $user->login);
      $url['query']['sess_tok'] = "$user->uid/$timestamp/$hash";
    }
  }
}

/**
 * Implements hook_boot().
 */
function pfizer_boot() {
  global $user;

  if (isset($_GET['sess_tok'])) {
    // If we are swithing between protocols, check whether the user has a valid
    // previous session first of all - if so then we won't need a full bootstrap.
    drupal_bootstrap(DRUPAL_BOOTSTRAP_SESSION);
    if (empty($user->uid)) {
      list($uid, $timestamp, $hash) = explode('/', $_GET['sess_tok']);
      $current = REQUEST_TIME;

      // Hardcoded timeout to a short period of time (60 sec) so that the url
      // doesn't stay valid for long.  In theory, this can be as low as the longest
      // amount of time it takes to load a page on the site, and should be as low
      // as possible.
      $timeout = 60;
      $account = db_select('users', 'u')
        ->fields('u')
        ->condition('u.status', 1)
        ->condition('u.uid', (int) $uid)
        ->execute()
        ->fetchObject();

      if (!empty($account->login)
          && ($current - $timestamp < $timeout)
          && !empty($account->uid)
          && ($timestamp >= $account->login || pfizer_is_hcp_user($account))
          && ($timestamp <= $current)) {
        // This should only happen once per user for the length of their session (first
        // time they switch between HTTP/HTTPS, so the forced bootstrap here is
        // acceptable.  Loading just user.module doesn't work well.
        drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
        // As above in pfizer_securepages_goto_alter(), don't check $account->login
        // for the HCP user, which is a common account that many people log into.
        $rehash = pfizer_is_hcp_user($account) ? user_pass_rehash($account->pass, $timestamp, $timestamp) : user_pass_rehash($account->pass, $timestamp, $account->login);
        if ($hash == $rehash) {
          // Check roles and refuse to log in admins because they'll always be on
          // SSL.  For Pfizer this is always true - only basic authenticated users
          // with no extra roles can be Federate/Pfizer Connect and HCP users.
          $new_user = user_load($uid);
          if (pfizer_user_basic_role($new_user)) {
            $user = $new_user;
            user_login_finalize();
          }
        }
      }
    }

    // Whether the user has a valid session or not, we need to remove the token
    // from the URL now and redirect to the actual destination.
    require_once DRUPAL_ROOT . '/includes/common.inc';
    require_once DRUPAL_ROOT . '/' . variable_get('path_inc', 'includes/path.inc');
    // If we only bootstrapped to session, then we need to initialize language
    // for url functions to behave, otherwise they generate notices.
    if (!isset($GLOBALS['language_url'])) {
       drupal_language_initialize();
    }
    $query = drupal_get_query_parameters();
    unset($query['sess_tok']);
    $options = !empty($query) ? array('query' => $query) : array();
    drupal_goto($_GET['q'], $options);
  }
}

/**
 * Determines if a given account is the Pfizer HCP common user.
 *
 * @param $account
 *   User object
 * @return
 *   TRUE if the passed-in user is the common HCP user, otherwise FALSE.
 *
 * @see _hcp_link_authenticator_get_username().
 */
function pfizer_is_hcp_user($account) {
  $hcp_user = variable_get('hcp_link_authenticator_user');
  // These should have both been saved by the system - no need to consider case
  // insensitive.
  if ($account->name == $hcp_user) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Determines if a user has only basic and no elevated roles.
 *
 * If anything other than the standard authenticated user role needs to be considered
 * here, it will need to be added to variable pfizer_basic_roles.
 *
 * @param $account
 *   User account object
 * @return
 *   TRUE if the account has no special roles.
 */
function pfizer_user_basic_role($account) {
  $pfizer_basic_roles = variable_get('pfizer_basic_roles', array(DRUPAL_AUTHENTICATED_RID));
  $extra_roles = array_diff(array_keys($account->roles), $pfizer_basic_roles);
  return empty($extra_roles);
}

/**
 * Implements hook_user_logout().
 *
 * We need to make sure the user gets logged out on both domains/protocols
 */
function pfizer_user_logout() {
  global $is_https;

  // This flag will be set if we arrive here from a logout on the other protocol.
  if (empty($_GET['ssllogout'])) {
    $url['path'] = 'user/logout';
    // If there is a destination set, we'll take that over here.
    $url['query'] = $_GET;
    // Add a flag to prevent redirect loops.
    $url['query']['ssllogout'] = 1;
    unset($url['query']['q']);
    // Additionally perform logout on the other protocol/domain.
    $url['https'] = !$is_https;
    $url['base_url'] = securepages_baseurl(!$is_https);
    $url['absolute'] = TRUE;
    $url['external'] = FALSE; // prevent an open redirect
    if (!empty($_GET['destination'])) {
      // Unset the current destination to avoid it overriding the path in drupal_goto().
      unset($_GET['destination']);
    }
    // Destroy session in the current protocol before redirecting to the other.
    session_destroy();

    drupal_goto($url['path'], $url);
  }
}

/**
 * Implements hook_drupal_goto_alter().
 *
 * Securepages does change the protocol but does not necessarily change the domain
 * if it is not performing a protocol switch - the domain is only changed when an
 * active protocol switch is needed, so we make the domain explicit here.  This
 * is needed specifically in our case to support the double logout
 */
function pfizer_drupal_goto_alter(&$path, &$options, &$http_response_code) {
  if (!isset($options['base_url'])) {
    // If we get here during boot, securepages is not loaded, but pfizer.profile
    // is.  We actually don't need to handle this case when securepages is not loaded
    // though, as arriving here during boot means that we redirected during boot,
    // which we assume is most likely from pfizer_boot().
    if (function_exists('securepages_baseurl')) {
      $options['base_url'] = securepages_baseurl(isset($options['https']) ? $options['https'] : $GLOBALS['is_https']);
    }
  }
}

/**
 * Implements hook_module_implements_alter().
 *
 * Make sure that the pfizer logout-related hooks fire last.
 */
function pfizer_module_implements_alter(&$implementations, $hook) {
  if (in_array($hook, array('user_logout', 'drupal_goto_alter')) && isset($implementations['pfizer'])) {
    $group = $implementations['pfizer'];
    unset($implementations['pfizer']);
    $implementations['pfizer'] = $group;
  }
}

/**
 * Implements hook_menu_alter().
 *
 * Ensure that anyone can access user/logout, even if not logged-in, because when
 * logging out cross-protocol, we don't know until the user arrives on the second
 * protocol whether they will have a valid session there also.  Allowing access to
 * user/logout is relatively harmless - just destroys session and redirects.  What
 * this does mean is that pfizer always need to manually hide the link when a user
 * is logged in, but they generally do not directly use this link anyway (due to
 * Federate needing a different logout link).
 */
function pfizer_menu_alter(&$items) {
  $items['user/logout']['access callback'] = TRUE;
  $items['user/logout']['page callback'] = 'pfizer_user_logout_custom';
}

/**
 * Perfoms logout operations only if we are logged in.
 *
 * Wrapper around gardens_client_user_logout_custom() to make sure we don't get
 * warnings when attempting to destroy uninitialized sessions.
 */
function pfizer_user_logout_custom() {
  if (!empty($GLOBALS['user']->uid)) {
    gardens_client_user_logout_custom();
  }
  drupal_goto();
}
