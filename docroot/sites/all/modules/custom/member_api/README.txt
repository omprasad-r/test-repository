The Member API module provides a Services-based REST API for user management on
Drupal Gardens sites.

The API uses two-legged oAuth. Traditional three-legged oAuth is a way for an
application developer to get credentials for an individual user's account, and
it involves sending the end user to a browser to enter application credentials
and approve access. Two-legged oAuth is so called because it leaves out the trip
to a browser-based authentication screen. It is simply a way for an application
to sign a request using an API key and secret. It is only useful if the
application developer has access to an API key corresponding to an
administrative user on the system. To use all the features of this API, you will
need an API key and secret belonging to a user with "administer users", "access
user profiles", and "administer permissions" permissions. Users with fewer
permissions will be able to use parts of the API. A Drush command is available
that will generate an API key for a given user id:

$ drush member_api-get-key <uid>

Signing an oAuth request
------------------------
There is a guide to signing oAuth requests at
http://hueniverse.com/oauth/guide/authentication/. It is highly recommended that
you use a library to sign requests, rather than trying to write an oAuth
consumer from scratch. The oAuth module includes the oauth-php library, which
can also be downloaded from http://code.google.com/p/oauth-php/.

Here is an example of how to make an oAuth GET request using this library:

<?php
require_once('OAuth.php');
$consumer = new OAuthConsumer('API key here', 'API secret here');
$signature_method = new OAuthSignatureMethod_HMAC_SHA1();
$path = 'http://yoursite.drupalgardens.com/memberapi/v1/user';

$request = OAuthRequest::from_consumer_and_token($consumer, NULL, 'GET', $path);
$request->sign_request($signature_method, $consumer, NULL);

$options = array(
  CURLOPT_RETURNTRANSFER => TRUE,
  CURLOPT_URL => $request->to_url(),
);

$ch = curl_init();
curl_setopt_array($ch, $options);
$result = curl_exec($ch);
$code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
print "Return code: $code\n\n";
print "Result: $result\n\n";
?>

Most POST and PUT requests will require sending multidimensional arrays. If
these are sent as application/x-www-form-urlencoded (i.e. name=abc&pass=123),
most OAuth clients will sign the request incorrectly. It is therefore
recommended to send the POST data as application/json. You can also use the
simple OAuthClient class included with this module, or look at it as an example
of how to send POST data as application/json.

Example of how to use the OAuthClient for a GET request:

<?php
require_once('OAuth.php');
require_once('OAuthClient.php');

$options = array(
  'endpoint' => 'http://yoursite.drupalgardens.com/memberapi/v1/',
  'key' => 'API key here',
  'secret' => 'API secret here',
  'path' => 'user.json',
);

$client = new OAuthClient($options);
$client->exec();
print "Return code: " . $client->getResultCode() . "\n\n";
print "Result: " . $client->getResult();
?>

Example of how to use the OAuthClient for a POST request:

<?php
require_once('OAuth.php');
require_once('OAuthClient.php');

$options = array(
  'endpoint' => 'http://yoursite.drupalgardens.com/memberapi/v1/',
  'key' => 'API key here',
  'secret' => 'API secret here',
  'path' => 'user.json',
  'httpMethod' => 'POST',
  'data' => array('name' => 'abc', 'pass' => '123', 'mail' => 'me@example.com')
);

$client = new OAuthClient($options);
$client->exec();
print "Return code: " . $client->getResultCode() . "\n\n";
print "Result: " . $client->getResult();
?>



How to use the API
------------------

In any of the examples below, "yoursite.drupalgardens.com" can be replaced with
any Gardens site domain. The ".json" ending can also be replaced with ".xml".


Retrieving a list of users
--------------------------
GET http://yoursite.drupalgardens.com/memberapi/v1/user.json?page=0&fields=uid,name

Expected response:
[
  {
    "uid": "5",
    "name": "user5",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/5"
  },
  {
    "uid": "4",
    "name": "user4",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/4"
  },
  {
    "uid": "3",
    "name": "user3",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/3"
  },
  {
    "uid": "2",
    "name": "user2",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/2"
  },
  {
    "uid": "1",
    "name": "admin",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/1"
  },
  {
    "uid": "0",
    "name": "",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/0"
  }
]


Retrieving users by email address (or other parameters)
-------------------------------------------------------
GET http://yoursite.drupalgardens.com/memberapi/v1/user.json?fields=uid,name,mail&parameters[mail]=user2@example.com

Expected response:
[
  {
    "uid": "2",
    "name": "user2",
    "mail": "user2@example.com",
    "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/2"
  },
]


Retrieving data on an existing user
-----------------------------------
GET http://yoursite.drupalgardens.com/memberapi/v1/user/1.json

Expected response:
{
  "uid": "1",
  "name": "admin",
  "pass": "21232f297a57a5a743894a0e4a801fc3",
  "mail": "admin@example.com",
  "mode": "0",
  "sort": "0",
  "threshold": "0",
  "theme": "",
  "signature": "",
  "signature_format": "0",
  "created": "1319482974",
  "access": "1319482991",
  "login": "1319482985",
  "status": "1",
  "timezone":null,
  "language": "",
  "picture": "",
  "init": "admin@example.com",
  "data": "a:0:{}",
  "roles": {
    "2": "authenticated user"
  }
}


Creating a new user
-------------------
Note: The POST data required to be sent can vary from site to site, depending
on what required user fields have been added. The example below is for a site
where field_birthdate is the only required user field.

POST http://yoursite.drupalgardens.com/memberapi/v1/user.json
Content-Type: application/json
{
  "name": "userA",
  "mail": "userA@example.com",
  "pass": "userApassword",
  "field_birthdate": {
    "und": [
      {
        "value": {
          "year": 1980,
          "month": 12,
          "day": 12,
          "hour": 12
        }
      }
    ]
  }
}

Expected response:
{
  "uid": "6",
  "uri": "http://yoursite.drupalgardens.com/memberapi/v1/user/6"
}


Updating an existing user
-------------------------
PUT http://yoursite.drupalgardens.com/memberapi/v1/user/6.json
Content-Type: application/json
{
  "name": "userA",
  "mail": "userAnew",
  "pass": "userAnewpassword"
}

Expected response:
{
  "name": "userAnew",
  "mail": "userAnew@example.com",
  "pass": "userAnewpassword",
  "uid": "6"
}


Deleting a user
---------------
DELETE http://yoursite.drupalgardens.com/memberapi/v1/user/6.json

Expected response:
true


Blocking a user
---------------
PUT http://yoursite.drupalgardens.com/memberapi/v1/user/6.json
Content-Type: application/json
{
  "status": 0
}

Expected response:
{
  "uid": "6",
  "roles": {
    "2": "authenticated user"
  }
  "status": "0",
}


Unblocking a user
---------------
PUT http://yoursite.drupalgardens.com/memberapi/v1/user/6.json
Content-Type: application/json
{
  "status": 1
}

Expected response:
{
  "uid": "6",
  "roles": {
    "2": "authenticated user"
  }
  "status": "1",
}


Retrieving a list of user roles
-------------------------------
GET http://yoursite.drupalgardens.com/memberapi/v1/roles.json

Expected response:
{
  "1": "anonymous user",
  "2": "authenticated user",
  "3": "administrator",
  "4": "site maintainer"
}


Retrieving a particular user's roles
------------------------------------
GET http://yoursite.drupalgardens.com/memberapi/v1/roles/6.json

Expected response:
{
  "uid": "6",
  "roles": {
    "2": "authenticated user",
    "4": "site maintainer"
  }
}


Updating a user's roles
-----------------------
PUT http://yoursite.drupalgardens.com/memberapi/v1/roles/6.json
Content-Type: application/json
{
  "roles": "2,3,4"
}

Expected response:
{
  "uid": "6",
  "roles": {
    "2": "authenticated user",
    "3": "administrator",
    "4": "site maintainer"
  }
}
