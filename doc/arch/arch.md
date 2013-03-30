[Home](../index.md)

Architecture Overview
=====================

A Drupal Gardens cluster typically comprises the following types of servers in varying numbers depending on the needs of the cluster. The terms **"cluster"** and **"stage"** are often used interchangably to describe all server resources used by a single gardens+gardener environment:

 - 1 master (this never changes)
 - a pair of web nodes (for the gardener)
 - multiple managed web nodes (for gardens sites)
 - multiple database pairs in master-master replication setup
 - a couple of backup servers (one acting as a *task server*)
 - an active load balancer, and a handful of hot spares
 - some fileservers

A subset of these can be chunked into "tangles" for stability and performance reasons.   A **"tangle"** then is an arbitrary subset of the cluster's managed webnodes, database clusters and fileservers. When a Gardens cluster reaches a certain size, its sites can't all be managed efficiently on the same infrastructure.  The limit practically speaking is dictated by gluster which starts to perform badly when a certain limit on total file size is exceeded. Because all sites on a *tangle* share a gluster mount, the storage used by all sites on a tangle counts against the gluster limit. At the point where gluster's effective operating capacity is approaching the limit, we create a new "tangle" and new sites and databases are created there from that point onward.  A tangle typically contains managed web nodes, database pairs and file servers (and is linked to load balancers).  A single gardener is used to manage sites in multiple tangles.

Each *tangle* in a Drupal Gardens cluster is essentially a massive *multi-site* installation where there is a single copy of the codebase, and individual sites are represented by subdirectories under the docroot/sites directory, which are implemented as symlinks to actual directories in the gluster filesystem (due to the need to share these directories between all web nodes serving a given site).

The *active tangle* is the tangle which is creating new sites. Each web-node in the *active tangle* asks the gardener constantly if there are any new sites to install, or any new sites to configure. Install is when we pre-install sites and configure is when a customer comes along and claims a site. See more in the [gardener/gardens](../gardens/gardens.md) documentation about site installation.

Master
------

The master holds the authoritative information about which sites are installed where, though it has minimal information about actual sites (eg. it has no idea what domain corresponds to which database) - it only really knows about things such as:

 - what machines of what type make up the cluster
 - which databases should exist on which db clusters
 - which actual database server instances belong to which cluster
 - which domains are assigned to which tangle
 - users' ssh access levels

Wherever there is a conflict between the gardener's idea of which tangle or database cluster a site is on, the master is considered to be correct.   There may be occasions where the 2 have conflicting information - some sites on the gardener will be missing this information from when it was not maintained, and on some sites it may be wrong if the site needed to be moved to a different database cluster for any reason.

In the more general hosting sense, a "tangle" or "gardener" is actually closer to the normal idea of what a "site" is.  The individual microsites in the multisite installation are more gardens-specific.

To get more detailed information about a site, it can be neccessary to correlate the database name known to the master with the site node on the gardener.  **The gardener site node ID corresponds to the database name for the site with a "g" prepended** eg the database name for site node nid *12345* is *g12345*.  Additionally, the site itself does not explicitly know it's own site node ID on the gardener, but it does know its database name, so can derive the node ID easily.

The master has a web UI which can be used to gather hosting information.  The url of the master always follows a consistent pattern:

`https://master.e.<stage name>.f.e2a.us/hosting`

Valid stage names (subject to change) include:

 - gardens (production drupalgardens.com)
 - gsteamer (drupalgardens.com staging)
 - fpmg-egardens (Florida hospital production)
 - wmg-egardens (Warner production)
 - enterprise-g1 (Pfizer production)
 - enterprise-g1-staging (Pfizer staging)
 - utest (used by utest for QA of new code before release)

Task server
-----------

Task servers distribute tasks between different services. Basically this is an endless loop implemented in Ruby. It checks for orders and once it receives one, it will fork and SSH into servers to run the matching commands (e.g. drush commands on Drupal sites).
