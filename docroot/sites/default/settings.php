<?php
/**
 * A user who gets here is trying to visit a site that is not yet registered
 * with either the Gardener or Hosting. Redirect them to an appropriate page on
 * the Gardener instead.
 */

// Don't run any of this code if we are drush
// or a CLI script.
if (function_exists('drush_main') || drupal_is_cli()) {
  return;
}

// Determine what stage we are in, so we can figure out if the user requested a
// custom domain or not. Use gsteamer as the fallback when we can't find a
// stage at all (e.g., local development machines).
$hostname = trim(`hostname`);
$components = explode('.', $hostname);
$stage = isset($components[1]) ? $components[1] : 'gsteamer';
$custom_suffixes = array(
  'gardens' => 'drupalgardens.com',
  'wmg-egardens' => 'wmg-gardens.com',
  'fpmg-egardens' => 'fpmg-drupalgardens.com',
  'enterprise-g1' => 'pfizer.edrupalgardens.com',

);
if (isset($custom_suffixes[$stage])) {
  $site_suffix = '.' . $custom_suffixes[$stage];
  $gardener_url = "http://www$site_suffix";
}
else {
  $site_suffix = ".$stage.acquia-sites.com";
  $gardener_url = "http://gardener$site_suffix";
}

// Now check the requested URL; if it doesn't end in the standard Gardens site
// suffix, it must be a custom domain.
$server_name = $_SERVER['SERVER_NAME'];
$is_custom_domain = !preg_match('/' . preg_quote($site_suffix) . '$/i', $server_name);

// Redirect to the correct page on the Gardener.
$redirect_url = $is_custom_domain ? "$gardener_url/domain-not-found" : "$gardener_url/site-not-found";
$location = $redirect_url . '?site=' . urlencode($server_name);
// Print a 404 response and a small HTML page explaining the redirect to the gardener.
// If the Refresh header is available in the browser, this will happen automatically and immediately.
header("HTTP/1.0 404 Not Found");
header('Content-type: text/html; charset=utf-8');
header('Refresh: 0; url=' . $location);
// If the Refresh header is not available, we print a tiny HTML page that redirects the user through javascript
// after 10 seconds. The link to the gardener page is provided in the case that the user does not have javascript enabled.
$javascript_timeout = 10000;
print <<<HTML
<!DOCTYPE html>
<html>
 <head>
  <meta charset="UTF-8" />
  <title>Drupal Gardens | 404 Page Not Found</title>
  <meta name="robots" content="noindex, nofollow, noarchive" />
  <script type="text/javascript">
    function delayedRedirect(){
      window.location = "$location"
    }
  </script>
 </head>
 <body onLoad="setTimeout('delayedRedirect()', $javascript_timeout)">
HTML;

print('<p>' . t('The Drupal Gardens site you are looking for cannot be found.') . '</p>');
print('<p>' . t('You will automatically be redirected to <a href="@redirect_url">@redirect_url</a> in @time seconds.', array('@redirect_url' => $location, '@time' => ($javascript_timeout / 1000))) . '</p>');

print <<<HTML
 </body>
</html>
HTML;
exit();
