SSL
===

SSL is not supported in gardens. (Well, it is somewhat supported, but it's best not to tell anyone about it).  Here are some things that can be done with SSL in gardens.


Wildcard certs
--------------

Currently the only SSL supported in gardens is a wildcard cert on the internal domain of the cluster - for example *.fpmg-drupalgardens.com.  This implementation has the implication that all SSL traffic has to use a domain matching the wildcard.  If a site normally uses a vanity domain and the user navigates to a page that uses SSL, they will be forced to the internal domain of the site.  The domains used for SSL and non-SSL are configured internally by the system when the securepages module is enabled.

The problem with this switching is that the user's session cookies will not be shared between the 2 domains and if they were logged-in on one domain, they might not be logged in on the other domain.  We have put a workaround in place on Pfizer for maintaining sessions between the 2 domains, but this is restricted to lower level users to limit the potential security implications.

The typical way to avoid this issue for this is to force all users to authenticate on SSL and force all authenticated traffic to SSL so there's no domain change for authenticated users.

*We are currently looking into options that might enable using vanity domains on SSL using multiple-domain UC certs on the load balancers.*

Note that if it were possible to support SSL on the vanity domain, it would most likely be neccessary to force the login page to SSL - it's generally possilbe to navigate using a session that was initiated on SSL to the insecure protocol, but it's not possible to log in on the insecure  protocol and navigate into SSL maintaining the session.

Secure Webforms
---------------
Enabling webform_ssl module will force all webforms to use SSL.  This is a common use case for SSL, to secure the data submitted in the form when sending over the web.


Secure roles and pages
----------------------

It is possible to force certain roles or pages to use SSL.  The **securepages** module is responsible for forcing these to SSL and back when the condition no longer applies. For example, it is possible to force all authenticated users onto SSL by setting securepages_roles to 

`array(DRUPAL_AUTHENTICATED_RID => DRUPAL_AUTHENTICATED_RID)`

You can force an individual page or several pages to SSL by setting securepages_pages to a newline-separated list of pages (which may user * as a wildcard).

Examples
--------

 - A fairly simple example of SSL config can be seen on Florida Hospitals' sites - all the configuration is done in securepages within the florida_hospital profile install file.  All authenticated users are forced to SSL.  The interesting thing to note here are are the various pages that must be forced to SSL for *all* users to ensure that no domain switching happens during authentication using username/pass on the site or openid.
 - A much more convoluted example is in the pfizer.profile file involving a combination of securepages configuration and hook_boot workarounds.
