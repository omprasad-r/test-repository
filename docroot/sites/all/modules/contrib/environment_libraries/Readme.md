Introduction
------------
Manage external css & jss through Drupal admin UI & Contexts. Files support
multiple versions based on prod/stage/dev environments for easier development
on production instances.

This module was developed by Interscope Records and designed with Acquia
Sitefactory in mind to accommodate building sites in a multisite environment
off a shared codebase without the need to push out updates for theme-level
updates.
It is based on [Javascript Libraries Manager](https://www.drupal.org/project/javascript_libraries).

##Feature List

*   Multiple File Libraries - Libraries can be comprised of multiple external
js & css files
*   Library Dependencies - Add dependencies to custom libraries from existing
enabled modules
*   Role-based Permissions - Only users with the
`view non-production environment_libraries` permission can view non-production
files.
*   Blocks - Load libraries by enabling blocks
*   Context Integration - Load js/css libraries via contexts
*   Features Integration - Export libraries via features


Installation
------------
*  Install as you would normally install a contributed Drupal module. See:
[https://drupal.org/documentation/install/modules-themes/modules-7] for further
information.


Recommended Modules
-------------------
*   (Recommended) Install the [context](https://www.drupal.org/project/context)
module to manage libraries
*   (Optional) Install the [chosen](https://www.drupal.org/project/chosen)
module for a cleaner UI to manage environment_library dependencies.


Configuration
-------------
Enable the `environment_libraries` module. Configuration options will
be available on the Configuration > System > Environment Libraries system
settings page.

## Settings
On the settings page you can select the current environment from the default
environments.
However, it is recommended that you use a custom integration using the module
hooks. see `environment_libraries.api.php`

You can also specify modules to search for library dependencies in. These
libraries (defined in `hook_library`) can be loaded via the UI.

## Managing Libraries
On the main listing page you can create or edit existing libraries.
Each library contains an external file location and options for each system
environment.

###Options:

*   Cache - Creates a local copy of the file in `public://environment_libraries`
 instead of loading it from the remote destination.
*   Aggregate - If cached, the file will be combined with other local files
*   Minification - (not yet supported)
*   Region / Weight - are used to determine the scope & order of the drupal
library. see [drupal_add_js](https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_add_js/7)
*   Dependencies - are other libraries that are required by the current library.
 Dependencies may be added even without files. This is how an existing library
 can be loaded with environment_libraries.

A library can be set to force a specific environment regardless of the
system environment set.
However, only user roles with the permission
`view non-production environment_libraries` will receive them.

## Loading Libraries
There are two primary ways to load a library on a page:
[context](https://www.drupal.org/project/context) and blocks.
The context integration is the recommended approach.

### Context
The context integration defines a reaction *Libraries* that can allows a library
 to be added to a context with whatever conditions you'd like.

### Blocks
The *expose block* option defines a block for the library that can be loaded
into a region to add the library to the page.


Troubleshooting
---------------
* If the configuration pages aren't displayed check that the
`administer environment_libraries settings` permission is set for the
current user role.


Permissions
----------
*   _administer environment_libraries settings_
*   _view non-production environment_libraries_

Variables
---------
*   _environment_libraries_environment_current_
*   _environment_libraries_module_dependencies_


Documentation
-------------
To generate html documentation files of the inline comments:

*   Install docco `sudo npm install -g docco`
*   Run `docco -e .php *.module *.inc *.install`
*   View `docs/environment_libraries.html`


Maintainers
-----------
*    Malcolm Poindexter (malcolm_p) https://www.drupal.org/u/malcolm_p
*    Ravindra Singh (RavindraSingh) https://www.drupal.org/u/ravindrasingh
