Statsd Dictionary
=================
This document contains a list of our statsd namespaces with a short description of the intent, and maybe what variations would be interesting when analyzing the data.

All entries should use the format "\[namespace\] ([type]) - [desc]" e.g. "user.sessions (counter) - Aggregate user sessions to watch for spikes which might indicate spam."

Antivirus
---------
- clamd.ok (counter) - The total number of scans that passed clam scan successfully.
- clamd.virus (counter) - The total number of scans that failed due to a found virus.
- clamd.error (counter) - The total number of failed scans.

Code Pushes
-----------

Site Operations
---------------
- site_operation.rate.preinstall (counter) - Number of site preinstallations.
- site_operation.rate.claim (counter) - Number of site installations which utilized a preinstalled site.
- site_operation.rate.create (counter) - Number of site installations which could not utilize a preinstalled site.
- site_operation.rate.duplicate (counter) - Number of site duplication requests.
- site_operation.rate.delete (counter) - Number of site deletions. Includes all deletions, so user initiated and reaped as well.
- site_operation.rate.fail (counter) - Number of site installation fails.

- site_operation.duration.preinstall (timer) - Number of milliseconds it took to preinstall a site.
- site_operation.duration.claim (timer) - Number of milliseconds it took to complete site installation starting from a preinstalled site.
- site_operation.duration.create (timer) - Number of milliseconds it took to complete site installation starting from scratches.
- site_operation.duration.duplicate (timer) - Number of milliseconds it took to duplicate a site.

- site_operation.states.provisioned (gauge) - Total number of provisioned sites.
- site_operation.states.installing (gauge) - Total number of sites being installed.
- site_operation.states.installed (gauge) - Total number of installed sites which have not yet been assigned to a customer.
- site_operation.states.reserved (gauge) - Total number of sites reserved for customers which have not been assigned so far.
- site_operation.states.assigning (gauge) - Total number of sites being assigned for customers.
- site_operation.states.duplicating (gauge) - Total number of sites being duplicated.
- site_operation.states.completed (gauge) - Total number of completed sites.
- site_operation.states.failed (gauge) - Total number of failed sites.
- site_operation.states.deleted (gauge) - Total number of deleted sites.

Reaper
------
- site_operation.rate.reaper (counter) - Number of reaped sites.
- site_operation.duration.reaper (timer) - Number of milliseconds it took to reap a site.
- site_operation.states.reaper_inactive.30 (gauge) - Approximate number of sites that have been inactive for 30 days.
- site_operation.states.reaper_inactive.60 (gauge) - Approximate number of sites that have been inactive for 60 days.
- site_operation.states.reaper.\[tangle name\] (gauge) - Number of sites queued for reaping for a given tangle.
- site_operation.states.reaper_failed (gauge) - Number of sites being reaped for too long. The initial definition of 'too long' is more than 20 minutes.

Theme Builder
-------------
- themebuilder.enter.success (counter) - Number of times the themebuilder has been entered.
- themebuilder.enter.fail (counter) - Number of times the themebuilder tried to open but failed.
- themebuilder.exit.success (counter) - Number of times the themebuilder was successfully opened.
- themebuilder.exit.fail (counter) - Number of times the themebuilder tried to exit and that failed.
- themebuilder.save (counter) - Number of times a save was done.
- themebuilder.save_as (counter) - Number of times a save as was done.
- themebuilder.publish (counter) - Number of times a theme was published
- themebuilder.enter (timer) - Time taken to open the themebuilder.
- themebuilder.exit (timer) - Time taken to close the themebuilder.


Theme Elves
-----------
- theme_elf.success.\[class name\] (counter) - Number of themes a given theme elf have fixed.
- theme_elf.fail.\[class name\] (counter) - Number of time a given theme elf failed to fix up broken themes.

User Sessions
-------------
- sessions.\[site name\] (gauge) - The total number of concurrent sessions (activity in the past 3 min).

User Registration
-----------------
- user.insert (counter) - Number of users added manually, automatically, etc.
- user.register (counter) - Number of users added by the user registration form.

XMLRPC
------
- rpc.error (counter) - Number of times the retry limit was reached.
- rpc.failure (counter) - Number of times the response failed.
- rpc.missingcreds (counter) - Number of times a gardens site was unable to retrieve credentials.
- rpc.missinghostname (counter) - Number of times the gardener hostname was missing from the credentials.
