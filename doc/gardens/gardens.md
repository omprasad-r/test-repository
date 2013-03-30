[Home](../index.md)

Gardens Overview
================

Gardens is an quick site creation platform hosted with Acquia's cloud hosting.

Gardener and Gardens sites
==========================

The sites themselves are called *Gardens sites*, while the site creation, user signup and central management platform is called *Gardener*. The small business platform (publicly visible on http://www.drupalgardens.com) is an instance of the Gardener. There are other instances of the Gardener for enterprise clients (which have their own set of gardens sites to manage). *Gardens* is a custom Drupal distribution with special features such as the themebuilder.

As of this writing, gardeners are Drupal 6 sites, gardens sites are Drupal 7 sites.

Gardens sites are set up and created in a multisite environment to make code sharing between them as seamless as possible. Sites have their own databases and uploaded files directories but cannot have their own set of extra modules in the base setup. Initial gardens sites are set up with *site templates* which are preconfigured sets of modules and settings (similar to Drupal install profiles but not implemented as such). Read more about [site templates](templates.md).

Gardener users have the power to create new sites and manage basic settings over their existing sites. There are different levels of service on differenr price points on the small business platform, see https://www.drupalgardens.com/pricing. Read more about implementation of [site management on the gardener](gardener_sites.md), [site access control on gardens sites](access_gardens.md) and [pricing based site limitations](limits.md).

Gardens sites do not communicate with each other, they communicate with the Gardener through different ways though, and the Gardener sends messages to sites in various ways, such as important values in response to [phone homes](phonehome.md). Task servers are used to fire tasks (such as drush tasks) on gardens sites. See more in the [architecture documentation](../arch/arch.md).
