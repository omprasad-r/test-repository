[Home](../index.md)

Site templates
==============

Gardens site templates are a concept similar to distributions, however, because the set of modules is wired in on Gardens, these templates are specific to Drupal Gardens (and not actually implemented with Drupal install profiles). Built-in templates include a campaign template, a blog template and a product template. These include combinations of features such as image galleries, webforms, forums, etc. as appropriate. It is also possible for users to create their own site setup by enabling certain features.

Templates offered on a gardener are set up with *site template* nodes. See admin/content/node (filter to *site templates*) for the nodes defined on the gardener. "Create your own template" is one of the templates offered. Site template nodes have fields for the features offered, under different groups such as "Features", "Pages", "Allowed features" and "Allowed pages".

The significance of templates from a marketing point of view is to show what Gardens can be capable of. From an engineering point of view, each template has a set of sites pre-installed, so if a pre-fabricated template is selected (and no customizations are made to the functionality), a site can be handed out right away to the user installing the new gardens site. That is the quickest way to get a Gardens site. (The number of pre-installed sites is managed at admin/settings/gardens_signup on the gardener.) If customized features are requested, a new fresh site needs to be installed, because site template options are not reversible as-is once installed.

Site templates are an install-only concept, once the site is installed, the regular Drupal modules and settings apply, there are no centralized feature management facilities on the site.

In the codebase, site template setup is managed by modules/acquia/site_template in the gardens repository.
