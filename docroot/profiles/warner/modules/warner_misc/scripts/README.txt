Current implementation:

1. Create /var/log/warner on managed servers (owned by tangle user)

2. Cron every five minutes on managed servers:
    */5 * * * * /usr/bin/php /var/www/html/tangle001/docroot/profiles/warner/modules/warner_misc/scripts/warner_stats_cron.php run

3. Cron once per day (just before midnight) on managed servers:
    55 23 * * *	/usr/bin/php /var/www/html/tangle001/docroot/profiles/warner/modules/warner_misc/scripts/warner_stats_cron.php flush

4a. Check log files /var/log/warner/warner_stats_[day of week - date("N")].log

4b. php /var/www/html/[tangle]/docroot/profiles/warner/modules/warner/misc/warner_stats_read.php [day of week - date("N")] [all]

