Enterprise Gardens Overview
===========================

Drupal Gardens began as a platform for creating Drupal websites quickly using just a web browser on drupalgardens.com and has since been modified to provide a whitelabel product allowing enterprise customers to have their own platforms for generating sites in a similar way to drupalgardens.com with some important differences.  Some examples:

 - enterprise gardeners do not use any payment handling as all the sites belong to the subscribing enterprise customer
 - all sites created on enterprise clusters are considered "gratis" sites - ie free full-featured sites 
 - the gardener theme is simplified on enterprise
 - most site management is done through the "site grid" at <gardener-host>/admin/gardens rather than "my sites"

Access
------
Typically, employees of an enterprise customer can never expect to be the highest level admin on their gardener.  There are 2 roles of interest:

 - platform admin: generally the highest level role that an enterprise customer can have, which still doesn't allow them to perform actions which could endanger the stability of their cluster (they cannot, for example, enable/disable modules, or make changes to site creation processes)
 - site builder: the lowest useful role on the gardener - for members of the customer's team responsible for building sites but who wouldn't need access to higher admin functions such as using the site manager for all sites on a cluster.

The highest level role on the gardener is "administrator" and is reserved for Acquia staff.


Gardener Theme
--------------

The gardener's main base theme is called ladybug, and the subtheme that produces styling for drupalgardens.com is called "orchid".  The default theme for enterprise gardeners is called "wallflower" and is also a subtheme of ladybug.



Install Profiles
----------------

Each enterprise customer *currently* has their own install profile under docroot/profiles.  The gardener has a variable set during installation of the enterprise gardener which tells the gardener to pass the name of the enterprise customer as a parameter to site installation.  This value can be changed on the gardener by super admins at <gardener-host>/admin/settings/whitelabel. The value of this variable must correspond to the name of an install profile directory in the gardens site codebase. Currently available profiles are:

 - Warner (warner)
 - Florida Hospitals (florida_hospital)
 - Pfizer (pfizer)
 - Gardens (gardens)

An **empty** value of the gardener install profile variable implies using the "gardens" install profile, which is used on drupalgardens.com (sometimes referred to as **SMB**: **S**mall and **M**edium-sized **B**usiness).  At some points in gardener code, the value of this variable is used to make enterprise-specific changes to the gardener.  Here, the empty value is used as an indicator that the site is either drupalgardens.com, or its staging cluster, *gsteamer*.

Each of these profile directories has a modules subdirectory that holds modules (or, potentially, specific *versions* of some modules) which are available only to sites installed under that profile - practically speaking, only available to the sites belonging to the enterprise customer named in the install profile.

When a site is installed with a given profile, it is first installed with the full *gardens* profile, and realtively late in the installation process the install profile is switched to the specific profile selected.  This is done so that all sites "inherit" features of the gardens profile, and customizations are then made to add additional features.  The gardens profile as a pseudo-module remains enabled on all sites.

It is possilbe to force a **test or staging** gardener to offer install profiles as an option during site creation. This is done by setting the variable *gardens_signup_enable_install_profile_selection* on the gardener to **TRUE**.  There is no UI for this, so it is recommended to do this on the gardener using the drush command:

`drush vset gardens_signup_enable_install_profile_selection 1`