[Home](../index.md)

Parature
========

Parature is a 3rd party service used to handle support requests for paid users on Drupal Gardens (including enterprise clients). On the Gardener, the gardener_help module provides a user interface to view tickets and submit tickets, while on Gardens sites, modules/acquia/gardens_help provides similar solutions.

Credentials are configured at admin/settings/parature_api on the Gardener which can be used to actually log in on the Parature user interface to check data as well.

The gardener keeps track of the service level of each user based on data from Parature. While the subscription level of each site is defined on the Gardener, that is sent through Zuora and sycned to Salesforce from there from where it gets sent to Parature, so it might take a little while after a subscription gets activated for the Gardener to be able to sync the servive level back. The Parature service level is maintained per user and loaded back from Parature, so support links are only operational if the Gardener believes the user will see useful things on the other end at Parature. (The error messages of Parature are misleading and useless). The Gardener's understanding of the user's Parature status and credentials can be seen on user/$uid/parature (where $uid is the user ID on Gardener). The full XML response from Parature is displayed and the processed data is displayed *and* updated.

The gardener_help module locally checks if the user is known to have Parature credentials and displays useful error messages in case of problems. If the user has permissions to go to Parature, it formulates a link directly to Parature, which in itself authenticates the user to Parature, so the login on the 3rd party service seems seamless.

Parature theming
----------------

The 3rd party Parature system uses some ungodly frames where some frames can have custom HTML and JavaScript but others only custom CSS. The looks of the Parature system is made similar to the Gardener with some tricks encoded mostly in the header frame JavaScript. Custom GET arguments such as dgEnterprise=0 and dgParent=www.drupalgardens.com are used to ensure proper logo display and backlink functionality in the header.

The support group can give credentials to modify those if needed. Parature does not keep any old versions of the customizations (no version control, no backups), so be careful when making changes.
