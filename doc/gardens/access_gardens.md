[Home](../index.md)

Gardens Site Access Control
===========================

Gardens sites are created with user #1 serving as the common *Gardens admin* account. The name of the account is literally *Gardens admin*. This allows support and gardens engineers to get into a site quickly without having a specific account on the site. The [gardens openid system](openid.md) lets users with proper permissions to log in to this account easily on any site if they are already logged in on the site's gardener with *Gardens admin*.

The actual site administrator gets a different user (possibly #3), and gets administration rights on the site (gets the administrators role). The site administrator is initially the same as on the site owner on the gardener. The user is assigned in the site assignment process (after site installation, when the site is assigned to a user). There are no provisions to keep the site owner on the gardener in sync with the gardens site admin and there can be multiple admins on the same permission level on the gardens site using Drupal's built-in capabilities.

The ultimate power of *Gardens admin* is that it is the most powerful user on every site and cannot be removed from the site. The site administrator has more [scarecrow limitations](scarecrow.md), not all of which apply to the Gardens admin account.

Although not strictly related to access control, there are also [site limitations](limits.md) enforced based on the pricing tier of a site (on the small business gardener).
