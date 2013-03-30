[Home](../index.md)

Pricing based site limitations
==============================

Drupal Gardens on the small business platform have pricing based site feature limitations. A publicly visible chart of those is at https://www.drupalgardens.com/pricing. Each pricing plan is represented on the Gardener with a node of type *Subscription product*, such as Starter, Professional, etc.

Subscription product nodes have a Product SKU as well a Product rate plan subSKU that is used to reference the subscription from Zuora and the product nodes themselves. Then the subscription tier controls whether local accounts are optionally allowed or what storage or webform limits are available or if the user can export their site. These subscription product nodes are also used to list disallowed modules on sites. The pricing table is generated from these data points.

The limitations are effective on the site after it [phoned home](phonehome.md) to the gardener. The response payload in the phone-home contains limits data related to the site's subscription. When the site is upgraded, a task server task is fired to tell the site to phone home to the gardener to update it's data about the limitations to apply.

The limitation data is accessible from gardens_client_data_get() and is managed by modules/acquia/gardens_limits with form alters and other wild tricks to make sure all limits are adhered to.

Not all limits are technically enforced as advertised.

Users get emails when certain limits are run over or are reached. The emails are configured at admin/settings/gardens-notifications on the gardener. Each site node has a boolean setting to suppress the overage emails named "Suppress overage notification emails".

Non-paid sites (free and gratis) are also subject to [reaping if inactive](reaper.md).

Paid sites are also entitled to support that we handle through a 3rd party service called [Parature](parature.md).
