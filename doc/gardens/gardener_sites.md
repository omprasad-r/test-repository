[Home](../index.md)

Gardener site management
========================

Gardener itself is a Drupal 6 site that lets users to manage their Gardens sites. Regular users on the small business platform can use their my-sites page (https://www.drupalgardens.com/mysites) to create new sites and manage existing sites. Admins and enterprise gardens users can use a more powerful views based site dashboard (located at https://www.drupalgardens.com/admin/gardens on the small business platform).

Each site is represented with a node on the Gardener of type "site". All the internal properties of the site are apparent when a site node is edited. This is only allowed for Gardens admins. Properties like the orignal site template, site features, the current site status, current site operation, subscription settings, mollom and janrain settings, and all site variables (used in case of enterprise) are on the node as regular CCK fields.

Site nodes are created when the site is initially created. Most sites start out life as preinstalled sites. These can be identified by several factors, their title will be in the pattern "Preinstalled site 50d1b3a2971cf3.90489855", their full URL be essentially invalid (http://g907241.gardenssite for example based on the node ID), and they are owned by "Gardens temporary site owner". The site name, URL, owner, etc. are filled in when the site gets an owner (as in when a user initiates a site creation operation). Preinstalled sites serve as a pool to take sites from quickly. The number of preinstalle sites are configured at admin/settings/gardens_signup on the gardener.

Site installation and configuration (when a user claims the site) are managed by the gardener in concert with the webnodes in the *active tangle* (see [the architecture documentation](../arch/arch.md) for more details). The webnodes on the active tangle are constantly polling for sites which need to be configured. The gardener just changes the fields of the site node (like title, etc) and changes the "current operation" field so the next poll will pick it up. When site configuration is done, the gardens site uses XML-RPC to tell the gardener that it is done. This updates the site node on the gardener, and the task bar for the user goes away when the site node's state has changed to completed.

When sites are taken by a user, the owner gets ownership over the site node. This probably has more significance on the small business system compared to the enterprise setups. The site owner sees the site on their mysites page and have permissions to do operations on the site.

The permissions are managed on a per-node basis with node level permissions using the content access module. See admin/content/node-type/site/access for site specific settings on the gardener. The *view* access, yes, the *view* access governs who has access to a site node. And access to a site node opens operations such as being able to delete the site, upgrade subscription, assign domains, duplicate, transfer ownership, site export, etc. This *view* access on the site node of course has no relation as to who can view the site itself.

Enterprise gardeners have a more flat permission structure and platform admins are permissioned to view the overall site management dashboard (/admin/gardens on a gardener site) and do operations on sites. Individual ownership of the sites has less significance.
