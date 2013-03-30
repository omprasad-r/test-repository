[Home](../index.md)

Site exports
============

Drupal Gardens is marketed as an OpenSaaS (Open Software as a Service) system, and part of the big promise is that you get a centrally hosted Drupal site which you can walk away with if you don't like the service anymore. Site export is a key feature. The export feature might also be useful to test sites locally in case of issues (and if the site is not preserved in scrubbing).

The site export functionality is accessible for users on their mysites page as one of the operation dropdown options. The actual export is happening on the gardens site and is managed by the modules/acquia/site_export module. When a site export is initiated, the user is asked questions about reasons, which is sent back to the gardener right away with a gardener XML-RPC call.

The site export process is responsible to protect certain internal values. Such as remove the OpenIDs for all users, reset the passwords for all users, remove modules (especially the [themebuilder](themebuilder.md), [scarecrow](scarecrow.md) and other gardens-only internal modules), and even undo some Gardens only changes that are applied to files. The logic for these operations are contained in modules/acquia/SiteExport.inc. Some examples:

 - SiteExport.inc has a list of files where stripped lines are removed starting from a marked ##GardensExcludeFromExportStart to a marker GardensExcludeFromExportEnd##. If you have lines to remove from files on export, mark them with these and include in the list in SiteExport.inc. The files listed here should also be listed in export_exclude.txt (so that the original version of the file is not exported, only this one).
 - getDirectoriesToRemove() lists all directories removed, such as modules/acquia wholesale, install profiles, etc.
 - getModulesToRemove() lists modules to remove; although this only allows modules in modules/acquia, and those files are already removed above, this list is used to remove database tables for these modules via their schema
 - sanitizeDB() removes session, OpenID, cache table contents, module variables, API keys, etc. (@todo document pitfalls)

For users, major drawbacks with site export include we no longer update their modules, they loose [the themebuilder](themebuilder.md) and all their users will need to be notified to ask for new passwords. We could not have the password intact technically anyway, since we don't have them on the gardens sites (and disclosing them would let exporters log into all other Gardens sites).

The site export functionality is also used when preserving sites that have been [reaped for inactivity](reaper.md).
