<?php
// http://www.mavetju.org/unix/netrc.php
// http://publib.boulder.ibm.com/infocenter/systems/index.jsp?topic=/com.ibm.aix.files/doc/aixfiles/netrc.htm
// http://net-netrc.rubyforge.org/

/**
Info on netrc format copied from rubyforge:

The .netrc file contains whitespace-separated tokens. Tokens
containing whitespace must be enclosed in double quotes. The following
tokens are recognized:

machine name
    Identifies a remote machine name. locate searches sequentially for
    a matching machine token. Once a match is found, subsequent tokens
    are processed until either EOF is reached or another machine (or
    default) token is parsed.
login name
    Identifies remote user name.
password string
    Supplies remote password.
account string
    Supplies an additional account password.
macdef name
    Begins a macro definition, which ends with the next blank line
    encountered. Ignored by Net::Netrc.
default
    Defines default account information. If supplied, default must
    appear after any machine entries.
*/

function gpdf_netrc_find_path($path = NULL) {
  if (!isset($path)) {
    $netrc = getenv('NETRC');
    $path = empty($netrc) ? getenv('HOME') . '/.netrc' : $netrc;
  }
  if (!file_exists($path)) {
    throw new Exception("No file at netrc path $path");
  }
  return $path;
}

/**
 * Try to retrive net credentials from the current user's .netrc file.
 *
 * @param $path = NULL
 *   The path to the .netrc file. The default is $_ENV['NETRC'] or
 *   $_ENV['HOME']/.netrc.
 * @return
 *   Array keyed by machine identifier with values of arrays with
 *   values with possible keys 'login', 'account', and 'password'.
 *
 * @throws Exception if netrc file is not found or invalid.
 */
function gpdf_netrc_read($path = NULL) {
  $path = gpdf_netrc_find_path($path);
  // @todo: the .netrc must be owned by the process’ effective user id
  // and must not be group- or world-writable, or a SecurityError will
  // be raised.
  $netrc = array();
  $lines = file($path);
  $machine = '';
  $inmacdef = FALSE;
  foreach ($lines as $line) {
    $line_content = (bool) trim($line);
    if (!$line_content) {
      // Exit macro upon empty line
      $inmacdef = FALSE;
      continue;
    }
    elseif ($inmacdef) {
      // Ignore macro definitions.
      continue;
    }
    $matches = array();
    preg_match_all('/("[^"]*"|\S+)/', $line, $matches);
    if (!empty($matches[0])) {
      // Remove leading/trailing ".
      foreach($matches[0] as $idx => $token) {
        $m2 = array();
        if (preg_match('/^"([^"]*)"$/', $token, $m2)) {
          $matches[0][$idx] = $m2[1];
        }
      }
      $first_tok = strtolower($matches[0][0]);
      switch ($first_tok) {
        case 'machine':
          // The 'default' must come last after all machine entries.
          if (isset($matches[0][1]) && $machine != 'default') {
            $machine = $matches[0][1];
          }
          else {
            throw new Exception("Invalid netrc format in line:\n$line");
          }
          break;
        case 'default':
          $machine = 'default';
          break;
        case 'login':
        case 'password':
        case 'account':
          if ($machine && isset($matches[0][1])) {
            $netrc[$machine][$first_tok] = $matches[0][1];
          }
          break;
        case 'macdef':
          $inmacdef = TRUE;
          break;
        default:
          break;
      }
    }
  }
  return $netrc;
}

