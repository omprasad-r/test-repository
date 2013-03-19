How to shut down the themebuilder.

---------------------------------------------------------------------------
Option 1 - ThemeBuilderMode file

This option is good for shutting down every site on a tangle without a
code change.  The effect is immediate, and there are several options
that provide some flexibility, such as allowing the themebuilder to be
used on a particular site when the rest of the tangle is shut down for
themebuilder use.

The ThemeBuilderMode scheme consists of a file placed at the top of
the workspace or in a site config directory or any combination to
affect either an entire tangle or a single site.  The ThemeBuilderMode
is a text file that consists of a single word that indicates what mode
the themebuilder should be in.  The valid modes are as follows:
--
"full" - Themebuilder can be opened and used

"locked" - Themebuilder cannot be opened, but can be used.  Note that
themebuilder will alert the user when the mode is set to locked to
indicate they should save their changes.  Their session will not ever
be forcibly closed in this mode.

"none" (or empty file) - Themebuilder cannot be opened or used.
Subsequent themebuilder requests will cause the themebuilder session
to close.
--

The ThemeBuilderMode file can be placed in the following locations:

DRUPAL_ROOT/../ - control behavior for an entire tangle
DRUPAL_ROOT/sites/<mysite>/ - control behavior for a single site

The single site configuration takes precedence over the tangle
configuration.  You can have the entire tangle enabled for
themebuilder use and a single site disabled or in maintenance mode, or
you can have the entire tangle disabled for themebuilder use with a
single site able to continue using the themebuilder.

-------------------------------------------------------------------------
Option 2 - use the themebuilder update level.

This option is good for disabling the themebuilder for each site until
the site has been updated.  This scheme uses a hardcoded value in the
source code and a value from the variable table to figure out if the
themebuilder should be accessible or not.  For the period of time
between the code update and the site update, the themebuilder will be
offline.

This is a source code solution that requires the value returned by the
themebuilder_compiler_get_update_level() function be incremented.
Additionally an update function must be written that calls
themebuilder_compiler_reset_update_level(), which causes the value in
the variable table to be updated to match that in the
themebuilder_compiler_get_update_level() function.

An example here is a previous update function that does the work:

/**
 * Reenable the themebuilder when the site has been updated.
 */
function themebuilder_compiler_update_7004() {
  themebuilder_compiler_reset_update_level();
}
