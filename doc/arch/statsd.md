statsd and graphite
===================
As part of our "house of data" philosophy, we are using statsd to push simple UDP requests to a common stats server. There is a statsd.js (nodejs) server collecting these and redirecting them to carbon/whisper - viewable with the graphite web interface. 

Installing statsd
-----------------
https://github.com/etsy/statsd

1. $ sudo apt-get install node # or similar package management
2. $ git clone git://github.com/etsy/statsd.git

Installing graphite
-------------------
I used a combination of the following two links. Though, the second one proved to be much more useful. There was one last step not mentioned though (noted below).

- http://graphite.wikidot.com/installation
- http://geek.michaelgrace.org/2011/09/how-to-install-graphite-on-ubuntu/[1]

**Additional graphite notes**

1. You might not need to pay attention to his mention of moving the "WSGIImportScript..." line in your apache conf. I didn't need to do this.
2. Make sure to set the following in your apache conf: "WSGISocketPrefix /var/run/wsgi" - I had to create that directory owned by root and set it in apache.
3. My first attempt was throwing exceptions so I reinstalled making sure I was on the 0.9.10 tag

Testing statsd/graphite
-----------------------
To do a quick test, you can use the following command to send a single UDP packet at your statsd server.

$ echo -n "test.dev.gauge:900|g"  | nc -w 1 -u 127.0.0.1 8125

gardens_statsd.module
---------------------
We decided to use our own statsd interface so that we can swap it out later if we decide, rather than peppering our codebase with dependencies on the d.o statsd module. If we decide to use the statsd module in the future, we can call it from our interface without worrying about dependencies. The interface for gardens_statsd is simple, containing one API function: gardens_statsd_send().

<?php 
gardens_statsd_send('my.stats.counter', 123, GARDENS_STATSD_COUNTER) 
gardens_statsd_send('my.stats.timer', 345, GARDENS_STATSD_TIMER) 
gardens_statsd_send('my.stats.gauge', 234, GARDENS_STATSD_GAUGE) 
?>

The path, specified in the first argument should be a dot-delimeted string. This will define the directory structure in the whisper db and subsequently the tree structure in graphite. The second argument is always an int which corresponds with the type of the statistic. Stats can be in one of three types, a counter, a timer or a gauge, specified by the third argument constant which evaluates to "c", "ms" or "g" respectively (part of the statsd standard). 

Interpreting Data
-----------------
Statsd does some clever things with the data that can make it difficult to quickly grok where your stats are saved. For example, a counter will be saved in two databases - one with the actual count and a second with a calculated "values per second" statistic. The former is found under "Graphite.stats_counts" in graphite while the later is found under "Graphite.stats". Gauges and Timers are saved under the "stats" section - e.g. Graphite.stats.gauges.your.gauge.
