[Home](../index.md)

Janrain
=======
Janrain provides social login and publishing capabilities, allowing users to log in with their Facebook, Twitter or other credentials, and post to their social accounts from within the site.

This is typically only available to enterprise customers.  A deployment on drupalgardens.com has been discussed but is not on the near term roadmap. [The drupalgardens.com site family uses OpenID](openid.md).


Janrain Engage
--------------
Janrain engage manages interactions with social authentication providers and social publishing and provides a single more or less consistent interface to interact with all of them.

There are 2 modules involved in Janrain Engage integration modules/acquia/janrain_client/janrain_client and janrain_login.  Janrain_login depends on janrain_client, but not the other way around.  It is possible to enable janrain for social publishing without enabling social login.

Instructions for setting up janrain for local development [borrowed from the intranet](https://i.acquia.com/wiki/testing-janrain-login-gardens-your-local-development-environment):

 1. Create a test site on a staging cluster such as *utest*. Make sure you create it as yourself, so that the site owner's email address is your email address, and make sure that the email address on your user account is a real email address that can receive mail. Also make sure the staging cluster is set up for local user registration; this is unlikely to work on an SMB site copied from gsteamer, for example.
 1. Make sure both "Social Login/Publish" modules (janrain_client.module and janrain_login.module) are enabled on the site.
 1. You may need to enable permissions to edit Janrain settings: https://skitch.com/jesse.beach/8dqi4/people-skrillex
 1. Make sure to enable at least one service (eg facebook or twitter) on this staging site so it will be available on the local test site.  The most important thing is not actually the service setup itself (as google is on by default) but to go through the Janrain account setup process against the staging site before importing to a local site.
 1. (optional?) You will need to set up a Twitter developer account. Maybe. Not entirely sure if this is necessary. You may want to create a fake Twitter account and mark it as private. When you add Twitter as a provider to your signin widget (on Janrain's site), you will be walked through the Twitter app creation process.
 1. Make a copy of the database for your staging site and import it into your local development site.
 1. Temporarily remove "display:none" from .ui-dialog .rpx-signin (either in themebuilder, or inspector/firebug, or in sites/all/modules/gardens_features/css/jquery.ui.dialog.css).
 1. Try to log into your local site via Janrain. This will fail.
 1. Check your email. You should have an email from Janrain warning you that someone tried logging into your site via a URL that was not whitelisted, and giving you instructions on how to whitelist it. Follow the instructions and add your local site to the Janrain whitelist.
 1. Try logging into your site again. It should work this time.

Note that the important part of the copied database is the Janrain variables, which are all stored in the variables table and start with "rpx\_". So if you need to test this on an existing site, where you can't start from scratch, follow step 1, then extract the new site's rpx_ variables into your existing site's variables table, then continue with step 3.

_Note - I somehow made the mistake of setting up a local site without clean_url on to import the rpx* variables into.  Janrain fails to load when this happens, so make sure clean_url is on._


The most typical issues when working with janrain are:

 - forgetting to whitelist all the domains that might use the app (including local dev domains like mydev.localhost)
 - janrain configuration variables getting scrubbed when production sites are duplicated.  Note that it is intentional that janrain variables are scrubbed when restoring production sites into staging to prevent staging sites using production apps.  It is possible to prevent janrain settings from being scrubbed on site duplication by checking the appropriate box at <gardener-host>/admin/settings/gardens_site_duplication


Gardener involvement
--------------------
The default Janrain Engage setup assumes that sites are independent, in charge of their own authentication and are set up with manually created Janrain apps.  This is not how it works in gardens.  For instance, if sites are set up to only use gardener openid for authentication, then all user accounts are actually on the gardener and janrain engage needs to log the user into the gardener which then logs the user into the site using openid.  It is also possible to configure the gardener with a "partner key" which can be used to create a new janrain app for each site as sites are created.

The module to be enabled on the gardener for this is called "Gardener Janrain" and its configuration options, including the partner API key are at <gardener-host>/admin/user/janrain.


Janrain Capture
---------------
Another option for integrating with Janrain is Janrain Capture.  In this variation, the majority of the user profile data is maintained in Janrain's database and only a stub of the user account is retained on the actual site in order to allow the user to log in.  The login box for the site is actually an iframe served by janrain, which then redirects the page to a token login url on the site on successful login.  At this point, some data can be sent to the site and mapped into the local user account.

It is not receommended to attempt enabling Capture and Engage simultaneously.  As far as I'm aware, it is not possible to use Capture with gardener openid login - user accounts need to be local.

The janrain_capture module is in sites/all/modules and is unmodified from the module hosted on drupal.org.

The setup for Capture is essentially just enabling the module and [entering credentials which are available on the intranet](https://i.acquia.com/wiki/configuring-janrain-capture-drupal-module).  There is an alternative login block to be enabled once this is done, and once setup is complete, it is also possible to configure capture to enforce all logins going through capture.  This should only be done once you're sure you can login through capture, otherwise you will be locked out of your site.


Janrain Federate
----------------
Janrain Federate is not strictly supported by Gardens but can be used.  It is essentially an extension of Janrain Engage that can maintain a user's session between multiple websites/domains (SSO).  Implementing Federate involves including a small Javascript that is best obtained from Janrain engineering.

It might be necessary in some engage deployments to prevent the default gardens javascript from being loaded in order to use a custom janrain javascript.  Setting janrain_login_add_default_js to FALSE on a site will prevent the default janrain javascript from loading.
