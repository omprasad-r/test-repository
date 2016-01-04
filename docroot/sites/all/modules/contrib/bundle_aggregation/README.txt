-- SUMMARY --

The bundle aggregation module provides an alternative solution to the core
JavaScript and CSS aggregation feature usable for high traffic sites. The intent
is to generate the aggregated bundles without waiting for locks or generating
high amount of disk writes while still serving all necessary files for the
visitors.


-- DESCRIPTION --

First of all, when the module is enabled it does not automatically take over of
existing aggregation features until it is activated on the configuration page so
it is possible to set up and test the aggregation feature without affecting the
existing functionality. To test the module while it is enabled but inactive the
'Use inactive bundle aggregation' permission is needed. Those who have this role
can visit pages with the bundle_aggregation a GET parameter
(http://www.example.com/?bundle_aggregation=1 for example)to invoke the custom
aggregation feature. The actual aggregation happens on queue processing so it
might take some time for the result to appear depending on the site settings.
This is also a good way to queue up bundles ahead of time so when the module
gets activated the visitors can be served with already existing bundles.

The bundle aggregation feature is similar to what core does:
 * gather files attached to the page,
 * group the files by their usual properties,
 * determine the bundle responsible for the given group using the group
   properties and page id,
 * attach files to the page
  * if the bundle exists then it will be attached to the page,
  * if there are files in the group which are missing from the bundle then the
    missing files will be attached to the page one by one without bundling,
  * queues up bundles that need regeneration.

Currently a bundle only gets sent off for regeneration automatically if a bundle
does not exist yet or a file is missing from it. Other changes, like a changed
file or a file that is no longer used anywhere, does not trigger automatic
regeneration. The easiest way to refresh bundles with file changes is to
increment the version number on the bundle settings page. Files which are no
longer used or have been moved can be removed from existing bundles with a form
on the bundle listing page.

If a site has multiple webheads and cache instances and want to distribute
the cache request load among the instances then
 * assign cache_bundle_aggregation bin on the webheads to a dedicated instance,
 * keep the cache_bundle_aggregation_locks bin assigned to globally accessible
   cache,
 * enable the 'Hierarchic caching' feature on the configuration page.

Things to keep in mind:
 * To be able to serve previous version of the bundles instead of generating one
   on the fly, the bundles are being identified using the CSS or JavaScript
   group properties plus a page id instead of the content of the file. This
   decision has made the bundle discovery more predictable, it also means that
   the bundle might could contain files which are not needed on the given
   pageload. For example a JavaScript which operates on a set of page elements
   without considering that the required elements might not be in place could
   block other JavaScript functionality due to erroring out.
 * If a bundle on a page has lots of missing files, it is possible that some
   browser might not handle all the individual attached files. This should only
   happen on a new kind of page or when the bundles have not yet been generated
   at all. This issue should clear up soon though once the queue processing has
   handled all the jobs.
 * If the pages end up without styling then there might be data in the database
   about the bundles but the files are missing from the webhead. Truncate the
   bundle_aggregation_bundles table, flush the whole cache_bundle_aggregation
   cache and it should start working.
 * If bundles would not generate no matter what, check that the target css and
   js directories have the proper permissions.


-- Planned features --
 * Option to regenerate bundles from scratch to get rid of unused files.
 * Option to detect changed files in a bundle and queue up for regeneration.
 * Option to generate bundles on pageview to support development needs.
 * Ability to tweak bundle grouping logic.
 * Ability to invoke aggregation with SESSION flag instead of GET parameter.


-- REQUIREMENTS --

None.


-- INSTALLATION --

Install as usual, see
http://http://drupal.org/documentation/install/modules-themes/modules-7 for
further information.


-- CONFIGURATION --

* Configure user permissions in admin/people/permissions.

 * Administer bundle aggregation settings

   This permission is needed to be able to activate or deactivate the bundle
   aggregation and to change the bundle version numbers to force the rebuilding
   of bundles.

 * Administer bundle aggregation technical settings

   Allows access to the more technical settings like enabling / disabling
   hierarchic cahing, setting how long does the bundle regeneration queue have
   to process jobs, etc.

 * Administer aggregated bundles

   This permission is required to be able to alter existing bundles like reorder
   or remove unused files.

 * Use inactive bundle aggregation

   When the bundle aggregation module is enabled by default the aggregation
   feature itself is inactivated. This permission allows the usage of the
   aggregation feature while it is inactive with a GET parameter.

* Configure up the bundle aggregation settings in
  admin/config/development/bundle-aggregation-settings.

* Change existing bundles at admin/config/development/bundle-aggregation.


-- CONTACT --

Current maintainers:
* Balazs Nagykekesi (nagba) - http://drupal.org/user/21231
* Marc Ingram (marcingy) - http://drupal.org/user/77320

This project has been sponsored by:
* Examiner
  http://www.examiner.com
