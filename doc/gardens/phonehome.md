[Home](../index.md)

Gardens phone-home system
=========================

A Gardens site can phone home to the gardener it belongs to. The *parent gardener* is configured in site install time, and the phone home happens in gardens_client_phone_home(). The phone home itself just sends along the site node ID (that the gardens site can tell from the database name that is applied to the site), the response contains various data bits about [site limitations](limits.md) that are vital for a site to behave according to the current subscription product relevant for the site.

The task server might instruct the gardens site to phone home (using the et_phone_home task) to gather updated data if needed. In effect instead of pushing down new data (since it has no way to do so), the gardener can tell the gardens site via the task server if there are changes that necessitate the phone-home.

gardens_client_phone_home() uses gardens_client_call_gardener() and there are other uses of calling the gardener that are not called phone-home but might be considered related. For example, gardens_client module uses it to send stats data once every day to the gardener about the gardens site (with gardens_client_send_site_data()). This includes number of users, nodes, comments, last node, last comment, etc. timestamp, last login, use of disk space, webforms, number of custom themes, extent of custom CSS, etc. Summaries of this data can be reviewed on different administrative views on the gardener such as at admin/reports/gardens/stats-view. This stats data serve as a major input for the [inactive free site reaper](reaper.md) that saves costs for us.
