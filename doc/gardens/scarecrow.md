[Home](../index.md)

Scarecrow hosting protection system
===================================

Scarecrow is the hosting protection layer of Gardens to hide and limit dangerous features from site owners as well as make sure all required gardens components are intact. Scarecrow is implemented via a module on gardens sites (modules/acquia/scarecrow) Examples of screcrow's functionality:

 - It hides gardens-required modules, so admins cannot disable them (including the screcrow module itself).
 - Hides in-development modules in production (or shows them in development environments)
 - Protects user 1 (Gardens admin), so it cannot be removed or edited
 - Ensures comments are either manually approved or mollom filtered
 - Hides the user login block and does other adjustments based on if OpenID is forced or not (flag set at install time)
 - Remove PHP execution capabilities such as Views PHP input
 - Denies access to various pages in the admin menu tree, such as the phpinfo page, akamai configuration and the views import screen (another PHP execution possibility)
 - And so on and on.

When new modules are added, it is likely that the scarecrow list needs updating for the new module, permissions, etc. Scarecrow's purpose is security, protection of the platform from mailicious users and accidental user mistakes. It also simplifies the platform a bit, however, pure simplifications where security is not involved are done in the gardens_features modules instead. Those simplifications (unlike scarecrow) are exported with [site exports](export.md).
