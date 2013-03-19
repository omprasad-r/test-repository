MASt Module

The MASt module provides Mobile App Services, with a focus on content creation
from a mobile app.

MASt provides endpoints via the Services module that describe your content types
so that any remote application (e.g., a mobile app) could retrieve enough
information to construct a content form.

At the moment, this only has an implementation for node entities, but the API is
generic enough to work with all fields and even arbitrary elements that you wish
to expose. The intention is to make this as generic and pluggable as possible so
that you can easily customize the endpoints in a consistent way.

Main features
-------------

1. Provide a UI to MASt-enable a content type. This is done by form altering the
   node_TYPE_form and adding a checkbox to the vertical tabs at the bottom. This
   can be added to other entity forms by replicating the submit handler.

2. Provide a default endpoint and resource to expose field data for a
   MASt-enabled content type, for example:

   http://example.com/content-types/fields.json

Installation
------------

Installing MASt module via drush enable will fail due to a naming issue with
OAuth module. You will need to download the OAuth module (not OAuth Common as
specified in the error message) before you can install MASt module via Drush.

This module should be used with the latest stable releases of Services
and OAuth, Services 7.x-3.3 and OAuth 7.x-3.0. There are minor API
changes in the Services module before 7.x-3.3 that will, at the least,
break the included tests if run against Services module before 7.x-3.3.

If you are running on PHP using fast-CGI, such as on Acqia Cloud, you need
to tweak your webserver configuration so that the content of the
Authorization header is accessible to the OAuth code. The easiest way to
do this is to patch .htacess. See issue  http://drupal.org/node/670454#comment-6439642
which has this patch: http://drupal.org/files/670454-12.patch

Mobile-friendly login page
--------------------------

The MASt module provides a mobile-friendly login page that can be used as an
alternative to user/login in case your site's theme is not mobile-friendly.

In order for this page to be used as the login page during the OAuth
authentication process, you need to have either the latest development version
of OAuth module, or version 3.0 with the patch from:

http://drupal.org/node/1710752#comment-6304372.

This makes the OAuth login path configurable using the
oauth_common_login_path variable. On installation, MASt module will then
automatically set this variable to 'mast/login', as long as it has not been
explicitly set already. This can be disabled from the MASt settings page.

To implement/override MASt classes
----------------------------------

1. Implement hook_mast_classes(). This function reports back the names of your
   classes that handle an entity bundle (type) as well as classes that handle
   your fields. A '*' can be used as a catch all for fields.

function mast_mast_classes() {
  return array(
    'entities' => array(
      'node' => 'mastEntityStructureNode',
    ),
    'fields' => array(
      'textfield' => 'mastFieldStandard',
      'image' => 'mastImageField',
      '*' => 'mastFieldStandard',
    ),
  );
}

   You can also implement hook_mast_classes_alter(&$classes) if you wish to
   manipulate these classes - specifically the ordering. The last class to be
   reported for any key will be used.

2. To create your own entity handling, extend the abstract mastEntity class. See
   mast.class.inc for details. Primarily, you need to implement the
   createFields() method as well as CRUD methods. See the mastEntityNode class
   for an example - this class uses createFields() to report entity fields as
   well as a few form elements that are coupled with nodes.

3. To create your own field types, extend the abstract mastField class. You will
   need to implement createField() and autoComplete(). The createField() method
   exposes meta data about the field that your remote application will need to
   construct a form (type, label, required, cardinality, etc.). autoComplete()
   returns an array of autocomplete options.

