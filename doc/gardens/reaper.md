[Home](../index.md)

Inactive site reaper
====================

Inactive non-paid sites are subject to a *reaping* process, where their content is archived on S3 and their active hosting is discontinued. Inactivity is measured based on last login data, site visitor statistics and so on, which is [collected daily from sites](phonehome.md).

The derivative inactivity data is collected on each site node. The "Inactive day counter" field indicates whether the reaper considers a free site active, inactive in the past 30, 60 or 90 days. "Inactive status" indicates whether the site was archived and "S3 Backup URL" is where it was archived at. When a site is reaped, it is not accessible anymore, but an exported archive is saved in S3, so the user can still take the site elsewhere. The archive is created with the [site export functionality](export.md).

Theoretically sites cannot be restored from this archive. Practically restorations can be made, but that is a rare exception and is a painful process (site nodes need to be restored, the setup for etc.).

The master switch for the reaper and the email settings are at admin/settings/gardens-notifications/inactivity and more detailed settings are at admin/settings/gardens-notifications/reaper with stats at admin/settings/gardens-notifications/reaper-stats.
