[Home](../index.md)

Themebuilder
============

The themebuilder is a unique Drupal Gardens feature and in many way is designed to run specifically in the Gardens codebase. A fundamental need for the themebuilder came from Gardens not allowing people to download and install custom themes, so their only device to customize their site looks is by using one of the Drupal Gardens base themes and diverging from that using the themebuilder functionality. The themebuilder is not exported on [site export](export.md) or [reaping](reaper.md).

Base themes used are the same across sites governed by the same Gardener and are located in themes/acquia in the gardens repository. The overall base theme is called builderbase, while some derivatives are called kenwood, broadway, impact, etc. Enterprise clients might have their own base themes, such as wmg located in the gardens respository as well.

Actual themebuilder created themes are saved in the filesystem at docroot/sites/$sitename/themes/mythemes.

Code for the themebuilder is located in the gardens respository as several modules under modules/acquia/themebuilder.

Themebuilder base services
--------------------------

The base module for the themebuilder is the themebuilder_bar module located in modules/acquia/themebuilder. This module defines the server callbacks for entering theme edit mode, exporting themes, etc. It also conceals all the built-in theme switching and configuration features of Drupal from the user, so all configuration and changes are done in the themebuilder.

Because theme changes involve file system level changes on the webnode, it is important that all theme operations happen consequtively on the same webnode. The themebuilder uses the 'ah_app_server' cookie to tie the theme editing user to a specific webnode (which later routing uses to make sure the user is sent to the right webnode). The themebuilder will refuse to edit the theme if there is a cookie mismatch.

Although the themebuilder_bar module handles the frontend request for saving themes and saving themes under new names, that is managed by the themebuilder_compiler module (just like export and update).

New themebuilder tabs can be defined with hook_themebuilder_bar_items() and hook_themebuilder_bar_tab_permissions(),

Themebuilder layouts
--------------------

Gardens does not include advanced layout capabilities, however some CSS tricks are offered for laying out regular blocks on pages (using classes swappable on the body tag). Each base theme has a possible set of layouts defined in their .info file in a configuration[] array such as 3-column "abc", "acb", "cab" layouts and 2 column "ac", "bc", "ca", "cb" combinations and one column setups. Layouts can be mapped to paths and path patterns. Defaults for these are set in the layout[] array in info files, such as forum and forum* paths mapped to a two column "ac" layout.

Users have the possibility to assign specific layouts to individual pages or all pages, where the base theme provided layout options are presented. Two column or one column layouts do not actually mean that the rest of the regions are not rendered, they are merely hidden with CSS.

Enterprise clients have a themebuilder_advanced_layout module where the layout to path mappings are editable in one big textarea, thsis is not exposed to small business clients.

TODO
----

More on how the Themebuilder works, session themes, passing data to the Themebuilder, Theme elves, versions, theme saving, backups, modifications.
