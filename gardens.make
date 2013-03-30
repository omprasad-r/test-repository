;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                                                   ;;
;;   Makefile for Drupal Gardens                                                                     ;;
;;                                                                                                   ;;
;;     Builds Drupal core and contrib modules, with patches.                                         ;;
;;                                                                                                   ;;
;;     PLEASE NOTE: This is still a work in progress. Please log issues and tasks in Jira:           ;;
;;     https://backlog.acquia.com/browse/DG-2161                                                     ;;
;;                                                                                                   ;;
;;     UPDATE [KB 4/13/2012]: In preparation for the switch to git we have made the necessary        ;;
;;     changes to this file that will at least allow it to run. The hope was that it would build     ;;
;;     the same codebase as is versioned in the docroot, however this is currently not the case.     ;;
;;     Although many of the changes are just $Id tags and packaging info, there are a large number   ;;
;;     of genuine code discrepancies between what the make file builds and what we have in svn.      ;;
;;     These will need to be eliminiated on a module by module basis. Any time you need to make      ;;
;;     a change to a contributed module, get the make file version in order; then gradually we will  ;;
;;     get it to the desired state.                                                                  ;;
;;                                                                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                                                   ;;
;;     NOTE: The "Drupal Gardens" user on drupal.org (creds:                                         ;;
;;     https://i.acquia.com/wiki/drupal-gardens-user-account-drupalorg) is setup to                  ;;
;;     follow all of the referenced issues in the file. When adding or removing d.o.                 ;;
;;     issue references from this file, also login to d.o. as that user and                          ;;
;;     follow/unfollow the corresponding issue.                                                      ;;
;;     TODO: Automate the following/unfollowing when changes to this file are made.                  ;;
;;                                                                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Core version
; ---------------------------------------
core = "7.x"

; API version
; ---------------------------------------
api = "2"

; Drupal core
; ---------------------------------------
;
; Current patches:
;
;  Patch to make robots go away on dev servers.
;    @see projects[drupal][patch][] = "https://raw.github.com/gist/3907652/ebf6efb432e1723681d36104ea1a55e7bdfc8cd7/dg-htaccess-changes2.patch"
;
;  Patch to perform a statcache on each request for drupalgardens multisite.
;    @see projects[drupal][patch][] = "https://raw.github.com/gist/dbfcd5198dd6df51f45f/53e8b86c5cbfaba18bc6f2e310037de9b25a3025/dg-multisite-statcache-workaround.patch"
;
;  Make it possible to re-use the internal logic of drupal_rewrite_settings() in other installation processes:
;    - AB 7/13/2010: http://drupal.org/node/852352#comment-3202542
;      - DR 10/22/2010: Patch still applies. This is kind of a feature request, so it might not get in. However, we can easily work around it (we only seem to be using it for the convenience of being able to easily reuse some core code in a custom Gardens Drush task).
;      - KS 4/21/2011: Updated to patch http://drupal.org/node/852352#comment-4378068
;      - BN 1/2/2013: Rerolling patch for Drupal 7.18 http://drupal.org/node/852352#comment-6894562
;    @see projects[drupal][patch][] = "http://drupal.org/files/drupal_rewrite_settings_d7-852352-17.patch"
;
;  Image module "scale and crop" effect forces upscale.
;    - AB 8/5/2010: http://drupal.org/node/872206#comment-3294646. Note, if that issue is bumped to D8, it can be solved in a contrib/custom module using hook_image_effect_info_alter().
;      - DR 10/22/2010: Patch still applies. This is kind of a feature request, so it might not get in. If not, we can deal with it as described above.
;      - KS 4/21/2011: Updated to patch http://drupal.org/node/872206#comment-4378294
;      - AB 10/25/2011: Updated to http://drupal.org/node/872206#comment-5160406
;    @see projects[drupal][patch][] = "http://drupal.org/files/872206-9.patch"
;
;  Block module should never disable all blocks for a theme
;    - DR 9/28/2010: http://drupal.org/node/925360 (first patch)
;      - DR 10/22/2010: Patch still applies. Seems like a bug whose fix would be committable at any time in the Drupal 7 release cycle.
;      - BN 30/08/2012: Updated to http://drupal.org/node/925360#comment-6414738
;    @see projects[drupal][patch][] = "http://drupal.org/files/prevent-disable-all-blocks-theme-925360-5.patch"
;
;  Support HTTP Authorization in CGI environment
;    - JS 11/18/2010: http://drupal.org/node/670454#comment-3731728
;     - PW 01/06/2010: note - this is needed for our tests to pass.
;     - AB 02/03/2011: Updated to http://drupal.org/node/670454#comment-4043230
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/bootstrap-http-auth-cgi-670454-11.patch"
;
;  Added --exclude option to run-tests.sh.
;    - JS 12/22/2010:
;      - DR 1/24/2011: I later added an -exclude-group option as well (and updated patches/run-tests-exclude.patch to include it).
;    @see projects[drupal][patch][] = "https://gist.github.com/raw/1071190cbafdb84a0526/3821a7186d572aad685b18204a303724d06bc1c5/run-tests-exclude.patch"
;
;  Added a patch to search tests to not throw exceptions in AH
;    - JS 2/14/2011: http://drupal.org/node/1061208
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/537278_searh_comment_toggle_quotes_2.patch"
;
;  Missing information about reason for OpenID login failure
;    - GH 3/02/2011: http://drupal.org/node/1078476#comment-4229028
;      - JB 11/15/2011: Updated to a newer version of the patch.
;      - DR 1/18/2012: Updated to most recent patch: http://drupal.org/node/1078476#comment-5463690
;    @see projects[drupal][patch][] = "http://drupal.org/files/openid_verbose_logging-1078476-17-d7.patch"
;
;  Make it possible to turn off admin email notification of new blocked users (so we are not spamming free tier customers as much.
;    - GH 4/05/2011: http://drupal.org/node/1116704 (first patch)
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/user_mail_register_pending_approval_admin_notify.diff"
;
;  Changed path_to_theme to drupal_get_path to resolve incorrect path when Seven is sub-themed
;    - JB 4/12/11: http://drupal.org/node/1125220#comment-4338664
;     - PL 8/8/2011: Updated patch - http://drupal.org/node/1125220#comment-4340210
;     - AB 10/25/2011: Updated patch - http://drupal.org/node/1125220#comment-5160490
;    @see projects[drupal][patch][] = "http://drupal.org/files/1125220-12-D7.patch"
;
;  Hidden, required modules cause a bogus confirmation message to appear when the modules page is submitted
;    - DR 6/30/11: http://drupal.org/node/1205684#comment-4677678
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/hidden-required-modules_0.patch"
;
;  ajax.inc empty status message results in empty div printed in DOM
;    - JB 8/2/11: http://drupal.org/node/1237012#comment-4813894, AN-27472
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/ajax_empty-status-message-1237012_1.patch"
;
;  Several important Core User settings lack support for hook_field_extra_fields()
;    - GH 9/8/2011: http://drupal.org/node/967566#comment-4882142, AN-27975
;      - AB 10/25/2011: Updated patch http://drupal.org/node/967566#comment-5152678
;    @see projects[drupal][patch][] = "http://drupal.org/files/967566-extra_fields-63.patch"
;
;  aggregator_aggregator_fetch() includes a watchdog and dsm call that references $feed->title, a value that is not guaranteed to exist.
;    - JB 11/10/2011: http://drupal.org/node/1337898#comment-5228594, DG-1453
;      - MS 12/27/2011: Reroll to apply with drush make
;    @see projects[drupal][patch][] = "http://drupal.org/files/aggregator_watchdog_feed_error-1337898-3.patch"
;
;  DrupalWebTestCase->getAbsoluteUrl should use internal browser's base URL
;    - DR 12/19/2011: http://drupal.org/node/471970#comment-5370470
;      - MS 12/27/2011: Reroll to apply with drush make
;    @see projects[drupal][patch][] = "http://drupal.org/files/471970-28.patch"
;
;  GET forms in the overlay were not supported, exposed filters in views under /admin are broke
;    - GH 1/11/2012: http://drupal.org/node/1116326#comment-4314820 (patch by KS)
;    @see projects[drupal][patch][] = "http://drupal.org/files/issues/1116326-4.overlay-method-get-forms.patch"
;
;  Make Syslog and Database Logging (dblog) configurable so we can filter out
;  unwanted messages and notices.
;    - BH 1/17/2012, Discussion here: http://drupal.org/node/1408208
;      - DR 1/25/2012: Updated patch to https://drupal.org/node/1408208#comment-5515832
;    - BN 30/8/2012: d.o issue became d8 specific, so rerolling https://drupal.org/files/syslog-dblog-filter-messages-1408208-7.patch as
;      https://raw.github.com/gist/f3438fb418057bdb008f/e2436e8733a3a1cb9f3e602b310fb823909aa665/syslog-dblog-filter-messages-1408208-7-reroll.patch
;    @see projects[drupal][patch][] = "https://raw.github.com/gist/f3438fb418057bdb008f/e2436e8733a3a1cb9f3e602b310fb823909aa665/syslog-dblog-filter-messages-1408208-7-reroll.patch"
;
;  Deleting a comment author while the Comment module is disabled leads to an EntityMalformedException error after it's reenabled
;    - DR 2/22/2012: http://drupal.org/node/1451072#comment-5640252
;    @see projects[drupal][patch][] = "http://drupal.org/files/comment-author-deleted-1451072-1-D7.patch"
;
;  Problem with function _field_invoke when used on a content type with no field instance
;    - DR 2/29/2012: http://drupal.org/node/1161708#comment-5669478
;    @see projects[drupal][patch][] = "http://drupal.org/files/1161708-field_invoke-trouble-19-D7-do-not-test.patch"
;
;  Unnecessary uses of $.each impacting frontend performance
;    - KB 3/14/2012: http://drupal.org/node/1428524#comment-5733382
;    - BN 1/3/2013: The issue has been fixed in D8 but was decided to not backport for D7 so we'll keep our version in gist.
;    @see projects[drupal][patch][] = "https://gist.github.com/raw/e3d5ab455f03efcce4af/5eafdaa87c3cf56121b858f8cc2512e5517759b6/core-js-replace-each-reroll-7-18.patch"
;
;  Problem with mailhandler running on multiple webnodes failing to process attachments of the same name.
;    - PL 3/14/2012: http://drupal.org/node/1482966#comment-5734092
;    (This module is in modules/acquia, so we don't use drush make to manage it yet)

;  Problem with mailhandler processing of single-part media emails
;    - AE 5/1/2012: http://drupal.org/node/1555792#comment-5940956
;    (This module is in modules/acquia, so we don't use drush make to manage it yet)
;
; 503 errors when files are in the database but not on disk
;   - PW 5/3/2012 http://drupal.org/node/1556396#comment-5953214
;     this should get in to core before long.
;   - GH 5/4/2012 http://drupal.org/node/1556396#comment-5955092
;     fixed PHP 5.2 compatibility issues
;
;  Overlay form submissions open in a new window when the form action is an absolute URL
;    - AE 5/23/2012 http://drupal.org/node/1082032#comment-6029794
;    @see projects[drupal][patch][] = "http://drupal.org/files/1082032-overlay-form-external-1-D7-do-not-test.patch"
;
;  Aggregator performance patch to solve long running queries on aggregator_item
;    table when there are lots of rows in it.
;   - BN 9/20/2012 http://drupal.org/node/1790298#comment-6501752
;   @see projects[drupal][patch][] = "http://drupal.org/files/add-index-to-aggregator-item-table-d7-1790298-5.patch"
;   - BN 10/4/2012 http://drupal.org/node/1676778#comment-6557912
;   @see http://drupal.org/files/drupal-ability_to_add_keywords_after_the_select_in_queries-1676778-5.patch
;   @see https://raw.github.com/gist/6efbd99fd2ed756e455b/2bdf0a6a4b55e9b4463a44339c2ac52d63c00be3/DG-5826-aggregator-performance.patch
;
;  Improper file API validation
;   - KH 10/16/2012 http://drupal.org/node/1815504#comment-6613890
;   @see http://drupal.org/files/file_save_upload_actual_uri_1815504_1.patch
;
projects[drupal][type] = "core"
projects[drupal][version] = "7.20"
projects[drupal][patch][] = "http://drupal.org/files/1556396-zombies-15-D7-do-not-test_0.patch"
projects[drupal][patch][] = "https://raw.github.com/gist/3907652/ebf6efb432e1723681d36104ea1a55e7bdfc8cd7/dg-htaccess-changes2.patch"
projects[drupal][patch][] = "https://raw.github.com/gist/dbfcd5198dd6df51f45f/53e8b86c5cbfaba18bc6f2e310037de9b25a3025/dg-multisite-statcache-workaround.patch"
projects[drupal][patch][] = "http://drupal.org/files/drupal_rewrite_settings_d7-852352-17.patch"
projects[drupal][patch][] = "http://drupal.org/files/872206-9.patch"
projects[drupal][patch][] = "http://drupal.org/files/prevent-disable-all-blocks-theme-925360-5.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/bootstrap-http-auth-cgi-670454-11.patch"
projects[drupal][patch][] = "https://gist.github.com/raw/1071190cbafdb84a0526/3821a7186d572aad685b18204a303724d06bc1c5/run-tests-exclude.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/537278_searh_comment_toggle_quotes_2.patch"
projects[drupal][patch][] = "http://drupal.org/files/openid_verbose_logging-1078476-17-d7.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/user_mail_register_pending_approval_admin_notify.diff"
projects[drupal][patch][] = "http://drupal.org/files/1125220-12-D7.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/hidden-required-modules_0.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/ajax_empty-status-message-1237012_1.patch"
projects[drupal][patch][] = "http://drupal.org/files/967566-extra_fields-63.patch"
projects[drupal][patch][] = "http://drupal.org/files/aggregator_watchdog_feed_error-1337898-3.patch"
projects[drupal][patch][] = "http://drupal.org/files/471970-28.patch"
projects[drupal][patch][] = "http://drupal.org/files/issues/1116326-4.overlay-method-get-forms.patch"
projects[drupal][patch][] = "https://raw.github.com/gist/f3438fb418057bdb008f/e2436e8733a3a1cb9f3e602b310fb823909aa665/syslog-dblog-filter-messages-1408208-7-reroll.patch"
projects[drupal][patch][] = "http://drupal.org/files/comment-author-deleted-1451072-1-D7.patch"
projects[drupal][patch][] = "http://drupal.org/files/1161708-field_invoke-trouble-19-D7-do-not-test.patch"
; Caches file_exists calls for aggregated css/js in apc. Pull request for pressflow here: https://github.com/pressflow/7/pull/16.
projects[drupal][patch][] = "https://raw.github.com/gist/c0867e74306d2c89bede/33d83a2a4492483a59f063450d93161e50597835/apc_cache_file_exists_aggregated_css_js.patch"
; Removes block cache/node_access restriction, provides a workaround for TAC.
projects[drupal][patch][] = "https://raw.github.com/gist/de25dfc8edfe03ebb7ee/2fe4f560a6963a2447b3a5aef0d89f19f3a4183f/block-cache-tac-workaround.patch"
; Saves and restores css/js when cached blocks are served.
projects[drupal][patch][] = "https://raw.github.com/gist/bb7c42876050a801714b/9b265d034098bf85ac6a511496ef92a2da47b818/1460766-block-cache.patch"
; JavaScript performance patch.
projects[drupal][patch][] = "https://gist.github.com/raw/e3d5ab455f03efcce4af/5eafdaa87c3cf56121b858f8cc2512e5517759b6/core-js-replace-each-reroll-7-18.patch"
; "Save configuration opens a new browser window"
projects[drupal][patch][] = "http://drupal.org/files/1082032-overlay-form-external-1-D7-do-not-test.patch"
; Changes that had been made to block.test rolled as gist
projects[drupal][patch][] = "https://raw.github.com/gist/c3138530aeaa01a9c7d7/0c6a7108b09e9aa9cd0dcde28d155120534a9eb1/block_test.patch"
; Add empty favicon
projects[drupal][patch][] = "https://raw.github.com/gist/3ddcf44817ceac4c35ef/ab3faf49a61420f86ab2a2262e3e3bb7f3e42038/empty_favicon.patch"
projects[drupal][patch][] = "http://drupal.org/files/1671318-5.hide-errors.D7-do-not-test.patch"
; Fix for fieldsets inside vertical tabs have no title and can't be collapsed
http://drupal.org/files/1015798-90-vertical-tab-legends_0.patch
; Aggregator performance patches
; http://drupal.org/node/1790298 contains a patch to add an index to
; aggregator_item but an other commit has taken up the update hook being used.
; While the patch gets in we'll use a gist to fix up the conflict.
projects[drupal][patch][] = "https://gist.github.com/raw/55ec61fd50a08327a85a/66ebeaf36847fa4e832371407bd22aad8ea076d1/add-index-to-aggregator-item-table-d717.patch"
projects[drupal][patch][] = "http://drupal.org/files/drupal-ability_to_add_keywords_after_the_select_in_queries-1676778-5.patch"
projects[drupal][patch][] = "https://raw.github.com/gist/6efbd99fd2ed756e455b/2bdf0a6a4b55e9b4463a44339c2ac52d63c00be3/DG-5826-aggregator-performance.patch"
; Improper file API validation
projects[drupal][patch][] = "http://drupal.org/files/file_save_upload_actual_uri_1815504_1.patch"
; Patch to .htaccess to allow shield module to work
projects[drupal][patch][] = "http://drupal.org/files/1343750-shield-fastcgi.patch"
; Field performance patch.
projects[drupal][patch][] = "http://drupal.org/files/field-info-cache-1040790-210-D7.patch"

; Custom module: improved_text_trim
; ---------------------------------------
;
;  Page gets cut down if there is an iframe in the teaser. (DG-3473)
;    - TD 03/27/2012: http://drupal.org/node/1504232
;    @see projects[improved_text_trim][patch][] = "http://drupal.org/files/improved_text_trim_iframe.patch"
;
projects[improved_text_trim][type] = "module"
projects[improved_text_trim][download][type] = "git"
projects[improved_text_trim][download][url] = "http://git.drupal.org/sandbox/effulgentsia/1378266.git"
projects[improved_text_trim][download][revision] = "99c1ad0c43c23e86028341d2c547573df86e64a5"
projects[improved_text_trim][patch][] = "http://drupal.org/files/improved_text_trim_iframe.patch"

; Contrib module: antivirus
; ---------------------------------------
projects[antivirus][version] = "1.0-alpha2"
; Patch to not scan remotely stored files.
projects[antivirus][patch][] = "http://drupal.org/files/antivirus-scans_fail_oembed-1872212-14.patch"
; Patch to add statsd tracking to antivirus actions.
projects[antivirus][patch][] = "https://raw.github.com/gist/4080782/c478eb3ead0fe5365f464b11f2acf38bc87a0ab7/antivirus_statsd.patch"


; Contrib module: acquia_connector
; ---------------------------------------
projects[acquia_connector][version] = "2.2"

; Contrib module: agrcache
; ---------------------------------------
;projects[agrcache][version] = "1.1"
;projects[agrcache][patch][] = "http://drupal.org/files/agrcache-prevent-merge-race-condition-1459526-3.patch"

; Contrib module: akamai
; ---------------------------------------
;projects[akamai][version] = "1.3"

; Contrib module: addthis
; ---------------------------------------
projects[addthis][version] = "2.1-beta1"

; Contrib module: audit_trail
; ---------------------------------------
;projects[audit_trail][version] = "1.x"

; Contrib module: backports
; ---------------------------------------
projects[backports][version] = "1.0-alpha1"

; Contrib module: colorbox
; ---------------------------------------
projects[colorbox][version] = "1.3"

; Contrib module: comment_goodness
; ---------------------------------------
projects[comment_goodness][version] = "1.4"

; Contrib module: comment_notify
; ---------------------------------------
projects[comment_notify][version] = "1.1"
; 2012-12-07 - KH - http://drupal.org/node/539214#comment-6091632 - This patch as been committed
; to the dev branch, so we could most likely remove it from here with the 1.2 release.
projects[comment_notify][patch][] = "http://drupal.org/files/539214_disable_notifications_more_broadly.patch"
; 2013-02-11 - KH - DG-6925 - http://drupal.org/node/1911870#comment-7042602
projects[comment_notify][patch][] = "http://drupal.org/files/comment_notify_node_notify_override_1911870_1.patch"

; Contrib module: contextual-flyout-links
; ---------------------------------------
;
; Note: The project name on drupal.org ("contextual-flyout-links") differs from the one currently
; used in Gardens ("contextual_flyout_links"). When we switch to building this module via Drush
; Make, we'll need to write a custom update function to move Gardens sites over to the new module.
;
projects[contextual-flyout-links][version] = "1.2"

; Contrib module: ctools
; ---------------------------------------
projects[ctools][version] = "1.1"

; Contrib module: date
; ---------------------------------------
;
projects[date][version] = "2.6"

; Contrib module: dialog
; ---------------------------------------
projects[dialog][type] = "module"
projects[dialog][download][type] = "git"
projects[dialog][download][revision] = "ba041e4b2a0114aa5dfe2f95f16e8dd2a4bba74e"
projects[dialog][download][url] = "http://git.drupal.org/project/dialog.git"
projects[dialog][patch][] = "http://drupal.org/files/user_forms_not_working_4_1348378.patch"
projects[dialog][patch][] = "http://drupal.org/files/dialog_ajax_error-1358624-3.patch"
; See DG-1901 and http://drupal.org/node/1348378#comment-5411722
projects[dialog][patch][] = "https://raw.github.com/gist/058f49fc2cb0165e2d2a/89dbd4e84c191a004c4da405e23b58bc301cd17d/dialog_user_access.patch"
projects[dialog][patch][] = "https://gist.github.com/anonymous/93acc7eeae3bedd4b8f8/raw/752129a40544a602dff28b5269cf5f07d67ebbb6/dialog_user_access2.patch"

; Contrib module: domain_301_redirect
;
; Current patches:
;
;  Ability to re-enable redirects that have been disabled by cron; Retry failed domain checks N times; Warn the user if cron has disabled redirection.
;    - KH 7/25/2012: http://drupal.org/files/1700414-3-verbose_and_rechecking_domain_availability.patch
;
; ---------------------------------------
projects[domain_301_redirect][version] = "1.2"

; Contrib module: edit_profile
; ---------------------------------------
projects[edit_profile][version] = "1.0-beta2"

; Contrib module: entity
; ---------------------------------------
projects[entity][version] = "1.0"

; Contrib module: entitycache
; ---------------------------------------
;projects[entitycache][version] = "1.1"
;projects[entitycache][patch][] = "http://drupal.org/files/entitycache-testcase-renaming.patch"

; Contrib module: entityreference
; ---------------------------------------
projects[entityreference][version] = "1.0"

; Contrib module: extlink
; ---------------------------------------
projects[extlink][version] = "1.12"
projects[extlink][patch][] = "http://drupal.org/files/1247644-language-and-ui-edits-4.patch"
; See http://drupal.org/node/953898#comment-3642016 (needed for 1329786 to apply)
projects[extlink][patch][] = "http://drupal.org/files/issues/extlink_area_tag.2.patch"
projects[extlink][patch][] = "http://drupal.org/files/1329786-7.extlink-js.patch"
projects[extlink][patch][] = "http://drupal.org/files/extlink-rejexvalidate-1434104-5.patch"

; Contrib module: extlink_extra
; ---------------------------------------
projects[extlink_extra][version] = "1.0-beta5"
; The following patch contains various pre-existing Gardens hacks and cleanup
; work that are not respresented in the drupal.org issue queue. Additionally,
; the patch from http://drupal.org/node/1846214#comment-6756938 had been added.
projects[extlink_extra][patch][] = "https://raw.github.com/gist/062012307daee069df08/979d85c7deaf2c5b6981a6ae97d770ff8537c251/extlink_extra.gardens.patch"

; Contrib module: feeds
; ---------------------------------------
;projects[feeds][version] = "2.0-alpha4"

; Contrib module: field_collection
;
;  Current patches:
;    - Field collection field handler for migrate module: http://drupal.org/node/1175082
;
; ---------------------------------------
projects[field_collection][version] = "1.0-beta5"
projects[field_collection][patch][] = "http://drupal.org/files/field_collection-migrate-1175082-194.patch"

; Contrib module: field_collection_table
; --------------------------------------
projects[field_collection_table][version] = "1.0-beta1"

; Contrib module: field_collection_views
; --------------------------------------
projects[field_collection_views][version] = "1.0-beta3"

; Contrib module: field_permissions
; ---------------------------------------
projects[field_permissions][version] = "1.0-beta1"
projects[field_permissions][patch][] = "http://drupal.org/files/field-permissions-create-1321050-6.patch"

; Contrib module: filter_tips_dialog
;
;   7.x-1.x at commit bb0d490dba8be9456a1b27cee86f642b23ba45db
;
; ---------------------------------------
projects[filter_tips_dialog][type] = "module"
projects[filter_tips_dialog][download][type] = "git"
projects[filter_tips_dialog][download][revision] = "bb0d490dba8be9456a1b27cee86f642b23ba45db"
projects[filter_tips_dialog][download][url] = "http://git.drupal.org/project/filter_tips_dialog.git"
projects[filter_tips_dialog][revision] = bb0d490dba8be9456a1b27cee86f642b23ba45db

; Contrib module: fivestar
; ---------------------------------------
;
; The git commit we are using is actually the 2.0-alpha2 tag, we are pulling it from git because
; there are patches affecting the .info file.

projects[fivestar][type] = "module"
projects[fivestar][download][type] = "git"
projects[fivestar][download][url] = "http://git.drupal.org/project/fivestar.git"
projects[fivestar][download][revision] = "19673ea159958c2d22e4ab496e0a58c781e4aaa6"

projects[fivestar][patch][] = "http://drupal.org/files/fivestar-restore_views_integration_for_votingapi-1509528-11.patch"
projects[fivestar][patch][] = "http://drupal.org/files/1526260-fivestar-microdata-entity-info.patch"
projects[fivestar][patch][] = "http://drupal.org/files/1537710-ajax-error.patch"
; http://drupal.org/node/1246656#comment-4854242, won't fixed on d.o
projects[fivestar][patch][] = "https://raw.github.com/gist/8fb8598aec1e407a06f4/eabbfe36edd8d65e706af802e8d61496afd1b1d0/1246656-new.patch"
projects[fivestar][patch][] = "http://drupal.org/files/1247388-2.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/78d8fcdd7d3ccb09eca2/935502a08c39e2f5d65fd599cfe543682b7e7cf6/1244196-hide-voting-target-config-on-field-settings-form.patch"
; http://drupal.org/node/1247614#comment-4857602, won't fixed on d.o
projects[fivestar][patch][] = "https://raw.github.com/gist/a63871a7e46c9ba7d795/70361b823e5e8ba44307c7a0c920bd5a37b9cbfd/gist-1247614-get-rid-of-configure-link.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/66ffd1cd268351afa9e5/e6639103620a58baaaad2f3967cfef51e39e8396/AN-27650.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/03db38f1f11eba3ffb44/51850a283a7abffa4754564a1de009e315af405a/AN-27664-revise-voting-tag.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/fd7b52aac636320922a0/2e08104474e7413e1f4588754afdee013b8402f0/AN-27704-delete-hooks.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/2a8fb9e1701881500354/0ea82f676f510341a1d33550e22f00a15dd4ad5c/fix-fivestar-update-7204.patch"
projects[fivestar][patch][] = "https://raw.github.com/gist/5bd855fcc100de77c592/11b3a211babee7f57289252d4985ff74d7f8d65f/fix-php-notice.patch"

; Contrib module: flag
; ---------------------------------------
projects[flag][version] = "2.0-beta9"

; Contrib module: flag_friend
; ---------------------------------------
projects[flag_friend][version] = "1.0-alpha9"

; Contrib module: flexible_blogs
; ---------------------------------------
projects[flexible_blogs][version] = "1.0"

; Contrib module: follow
; ---------------------------------------
projects[follow][version] = "1.0-alpha1"

; Contrib module: form_builder
; ---------------------------------------
;
; Current patches:
;
; form_builder_pre_render is run after drupal_pre_render_markup
;  - JE 5/18/2011: http://drupal.org/node/1161666
;    - AB 12/28/2011: This patch has been committed to 7.x-1.x.
;  @see http://drupal.org/files/issues/0002-form_builder_pre_render-is-run-after-drupal_pre_rend.patch
;
; Webform integration uses non-existant Webform label options
;  - DR 6/21/2011: http://drupal.org/node/1149734
;    - DR 6/21/2011: This patch has already been committed to 7.x-1.x.
;  @see http://drupal.org/files/issues/form_builder_webform_labels-d7.patch
;
; Webform select elements with no default value don't show "None" when added via the form builder
;  - DR 6/23/2011: http://drupal.org/node/1198136 (first patch)
;   - AB 12/30/2011: Updated to http://drupal.org/node/1198136#comment-5416676
;  @see http://drupal.org/files/form_builder-empty-value-select-1198136-1.patch
;
; Webform integration does not show correct title display options for textareas or grids
;  - DR 7/20/2011: http://drupal.org/node/1223880#comment-4759054
;  @see http://drupal.org/files/issues/form-builder-webform-title-display-inline-1223880-2.patch
;
; Form Builder causes Notice: Undefined index: weight in _registry_parse_files()
;  - AB 12/28/2011: http://drupal.org/node/1153596#comment-4456156
;    - AB 12/28/2011: This patch has been committed to 7.x-1.x.
;  @see http://drupal.org/files/issues/form_builder_registry_weight.patch
;
; Combined JS and CSS patches includes following issues
;  - AB 12/30/2011: http://drupal.org/node/1033462#comment-5416694
;  @see http://drupal.org/files/form_builder-webform_alt_ui-js.patch
;  @see http://drupal.org/files/form_builder-webform_alt_ui-css.patch
;  @todo Includes fallout from the following issues:
;
;        Allow other modules to specify where the field configuration form will be displayed
;          - JE 2/1/2011: http://drupal.org/node/1047612#comment-4033140
;            - DR 6/21/2011: quicksketch posted an updated patch (http://drupal.org/node/1047612#comment-4447900) which made a number of changes to ours. Most of them look OK, but this still needs a more careful review before we take it in.
;          @see http://drupal.org/files/issues/form_builder_custom_form-d7_1.patch
;
;        Improve AJAX form requests and submission
;          - DR 2/1/2011: http://drupal.org/node/1010926
;            - DR 6/21/2011: Patch is currently marked needs work. It also no longer applies, so I posted a revised version (http://drupal.org/node/1010926#comment-4633348). Also note that this patch conflicts with the one listed above by quicksketch (http://drupal.org/node/1047612#comment-4447900), although the conflicts are relatively minor.
;          @see http://drupal.org/files/issues/form-builder-ajax-1010926-2.patch
;
;        Dragging fields into fieldsets is wonky and need refinement
;          - DR 2/1/2011: http://drupal.org/node/1011060
;            - DR 6/21/2011: This patch has been committed to 7.x-1.x, but with some major changes that need review. Essentially all the CSS changes in the patch are gone, and the JavaScript changes seem pretty significantly rewritten also.
;          @see http://drupal.org/files/issues/form_builder_dragdrop.patch
;
;        We appear to have some other undocumented changes to this module besides those listed above.
;        Most (hopefully all?) of these are included in the catch-all patch I posted
;        at http://drupal.org/node/1033462#comment-4013352 a while ago, but do not appear to be posted
;        anywhere in the Form Builder issue queue as actual independent issues (or if they are, they weren't
;        ever listed here). Some of that patch overlaps with issue http://drupal.org/node/1047612 from above,
;        though, "Allow other modules to specify where the field configuration form will be displayed".
;          - DR 6/21/2011: http://drupal.org/node/1033462#comment-4013352
;          @see http://drupal.org/files/issues/form-builder-js-extra-webform-alt-ui-2.patch
;
; Call to undefined - function form_builder_get_form_type()
; - DR 1/30/2012: Added http://drupal.org/node/1203848#comment-5311484 to Gardens to prevent fatal errors when the Form Builder field palette block is enabled. This patch has already been committed to the Form Builder project.
; @see projects[form_builder][patch][] = "http://drupal.org/files/form_builder-block-error-1203848-2.patch"
;
projects[form_builder][version] = "0.9"
projects[form_builder][patch][] = "http://drupal.org/files/issues/0002-form_builder_pre_render-is-run-after-drupal_pre_rend.patch"
projects[form_builder][patch][] = "http://drupal.org/files/issues/form_builder_webform_labels-d7.patch"
projects[form_builder][patch][] = "http://drupal.org/files/form_builder-empty-value-select-1198136-1.patch"
projects[form_builder][patch][] = "http://drupal.org/files/issues/form-builder-webform-title-display-inline-1223880-2.patch"
projects[form_builder][patch][] = "http://drupal.org/files/issues/form_builder_registry_weight.patch"
projects[form_builder][patch][] = "http://drupal.org/files/form_builder-webform_alt_ui-js.patch"
projects[form_builder][patch][] = "http://drupal.org/files/form_builder-webform_alt_ui-css.patch"
projects[form_builder][patch][] = "http://drupal.org/files/form_builder-block-error-1203848-2.patch"

; Contrib module: getid3
; ---------------------------------------
projects[getid3][version] = "1.0"

; Contrib module: google_analytics
; ---------------------------------------
projects[google_analytics][version] = "1.2"

; Contrib module: htmlpurifier
; ---------------------------------------
;
; Current patches:
;
;  Performance modification consisting of not calling variable_set every time the user saves a node.
;    - AB 02/03/2011: http://drupal.org/node/993276 (first patch). Patch was committed to HEAD, but not backported to this branch
;    @see projects[htmlpurifier][patch][] = "http://drupal.org/files/issues/htmlpurifier_variable_set.patch"
;
;  Remove double caching logic
;    - PL 1/17/2011: http://drupal.org/node/993274 (first patch)
;     - AB 02/03/2011: Patch was committed to HEAD, but not backported to this branch
;     @see projects[htmlpurifier][patch][] = "http://drupal.org/files/issues/htmlpurifier_cache.patch"
;
; ---------------------
;
; Notes:
;
;  AN-26983 - update html_purifier to 2.x
;
;  DR 6/24/2011: NOTES ABOUT THINGS TO CHECK WHEN UPGRADING TO 7.x-2.x (besides these, the upgrade diff looks perfectly safe):
;    - Need to examine http://drupalcode.org/project/htmlpurifier.git/commitdiff/58c4001 (http://drupal.org/node/925306) for its effect on our desired security level in Gardens.
;    - Need to examine http://drupalcode.org/project/htmlpurifier.git/commitdiff/82a9efa (looks correct, but I'm not sure it's backwards-compatible with settings that Gardens sites have stored in the database or if it needs a Gardens-specific update function).
;    - Need to look into http://drupal.org/node/1195294 (missing file on the 7.x-2.x branch, though I think it might not affect us in Gardens).
;
; ---------------------

projects[htmlpurifier][type] = "module"
projects[htmlpurifier][download][type] = "git"
projects[htmlpurifier][download][url] = "http://git.drupal.org/project/htmlpurifier.git"
projects[htmlpurifier][download][revision] = "72f28273bb6e4616ed9f223bae2dc393f01dc5b2"
projects[htmlpurifier][patch][] = "http://drupal.org/files/issues/htmlpurifier_cache.patch"
projects[htmlpurifier][patch][] = "http://drupal.org/files/issues/htmlpurifier_variable_set.patch"

; Contrib module: janrain_capture
;
; Current patches:
;
;   Code consistency patch.
;     - BN 20/03/2013 http://drupal.org/node/1947698
; ---------------------------------------
projects[janrain_capture][version] = "2.0-rc3"
projects[janrain_capture][patch][] = "http://drupal.org/files/code-consistency-fix-1947698-2.patch"

; Contrib module: javascript_libraries
;
; ---------------------------------------
projects[javascript_libraries][version] = "1.2"


; Contrib module: job_scheduler
; ---------------------------------------
; projects[job_scheduler][version] = "2.0-alpha2"

; Contrib module: libraries
; ---------------------------------------
projects[libraries][version] = "2.0"

; Contrib module: link
; ---------------------------------------
projects[link][version] = "1.0"
; http://drupal.org/node/1409980#comment-5491670
projects[link][patch][] = "http://drupal.org/files/link-validation.patch"

; Contrib module: mailing_list
; ---------------------------------------
projects[mailing_list][version] = "1.0-beta1"
projects[mailing_list][patch][] = "http://drupal.org/files/issues/fix-notice-watchdog_0.patch"

; Contrib module: MASt
; ---------------------------------------
projects[mast][version] = "1.0-rc1"

; Contrib module: media
; ---------------------------------------

projects[media][version] = "1.0"
projects[media][patch][] = "http://drupal.org/files/1835164.media_.token_support.patch"
projects[media][patch][] = "http://drupal.org/files/1512258_media_dpm.patch"
; [KB 12/11/07] Rerolled to apply on top of the token support patch
projects[media][patch][] = "https://raw.github.com/gist/8994280b2e7fe87a5dfe/614ced9f725d425c695c55ec69dd7e6f7f5d6f29/media-default_formatter.patch"
projects[media][patch][] = "https://raw.github.com/gist/2877930/a0dfaea201a505548ed9be7154163dc4e0169c0e/media-edit_form_styling.patch"
projects[media][patch][] = "https://gist.github.com/raw/c799a0cfa6b8e807a3bc/653748325182df8e424846018809a57028fd46f2/media-file_view_file-alter.patch"
projects[media][patch][] = "https://gist.github.com/raw/1323359cee78b9ef7491/ed8aa5109245e30ca92c75dde7556e5443748332/media-manage_files_breadcrumb.patch"
; http://drupal.org/node/1278180#comment-5085370
projects[media][patch][] = "http://drupal.org/files/media-embed-library-resize.patch"
projects[media][patch][] = "http://drupal.org/files/expose-file-types-to-wysiwyg-1016376-58.patch"
projects[media][patch][] = "https://raw.github.com/gist/1722887/1e5f99c5e30d3ccf8798be31d5032de8bbf93e3b/media-hide-addfile-nonjs-followup-1238298-8-REROLL.patch"
; The following patch was applied to the 7.x-1.x and the 7.x-2.x branches of the Media module. Since we
; are running a version of the module with this patch applied to our codebase, I need to augment the patch
; in order to make the changes necessary to address DG-4778. The new patch file is stored as a gist.
; - projects[media][patch][] = "http://drupal.org/files/1307596-12.browser-validation-behavior-1.x.patch"
projects[media][patch][] = "https://raw.github.com/gist/3239414/92725c8abb9520cd7ab7cf739557da8d693e1ca2/gistfile1.txt"
projects[media][patch][] = "http://drupal.org/files/1266064-media-infinite-recursion-4.patch"
projects[media][patch][] = "http://drupal.org/files/1591534-5.patch"
projects[media][patch][] = "https://raw.github.com/gist/14fa0a08a1f719e65d11/3e56fc138aa127500b45661252f455ee2792a796/media_warn_delete-1461260-gist-dg2660.patch"
projects[media][patch][] = "http://drupal.org/files/1364640.media_popup_js.3.patch"
projects[media][patch][] = "http://drupal.org/files/1595194_media_ie-image-upload-error_2.patch"
projects[media][patch][] = "http://drupal.org/files/no-media-available-1500292-4.patch"
projects[media][patch][] = "http://drupal.org/files/1565448-empty-browser-tab-output.patch"
projects[media][patch][] = "https://raw.github.com/gist/d597bb08e956ac5cb6a8/4528fcfea2afa74d6b0c86818f2eeb35f5b4a7e0/DG-4776.remove-overlay-borders.patch"

; Contrib module: media_browser_plus
; ---------------------------------------
projects[media_browser_plus][version] = "1.0-beta3"
; See https://github.com/acquia/gardens/commit/2abb5e7eeaf312b8c5df988888d2c88cc0d90618
projects[media_browser_plus][patch][] = "https://raw.github.com/gist/2052bade298a70fe72cc/b1b3fd81ca5ee72e7b3d797edd8bc1a74bba84e6/DG-150_media_browser_plus.patch"

; Contrib module: media_crop
; ---------------------------------------
projects[media_crop][version] = "1.4"

; Contrib module: media_gallery
; ---------------------------------------
projects[media_gallery][version] = "1.0-beta8"
; http://drupal.org/node/1433558
projects[media_gallery][patch][] = "http://drupal.org/files/media-gallery-entity-malformed-exception.patch"
projects[media_gallery][patch][] = "http://drupal.org/files/1585864_missing-media-gallery-vocab_2.patch"

; Contrib module: media_youtube
; ---------------------------------------
;
; Current patches:
;
;  Combined patch for all issues below:
;    MS 12/29/11
;    @see projects[media_youtube][patch][] = "https://raw.github.com/gist/5d8c97a21cd40b23a5e1/e477b036c58b68d65b7f25a0e27c6900ec0076e8/media_youtube-combined-diff.patch"
;
;    Add a link to the YouTube video below the thumbnail on the media edit form
;      - AB 10/25: http://drupal.org/node/952638
;        - AB 06/29/2011: Original issue moved to Media queue for more general solution. Meanwhile, rerolled patch is in patches/media_youtube-extra-changes.patch.
;      @see projects[media_youtube][patch][] = "https://gist.github.com/raw/e8ceeb20e3edc42a3aeb/f19a12f201f5b04b2ba4c3efb35883aab89e0680/media_youtube-extra-changes.patch"
;
;    Videos no longer resize themselves to fit their containers
;      - JE 3/31/2011: http://drupal.org/node/1112462
;        - AB 06/29/2011: Rerolled: http://drupal.org/node/1112462#comment-4673310
;        - JE 09/13/2011: Rerolled: http://drupal.org/node/1112462#comment-4984178
;      @see projects[media_youtube][patch][] = "http://drupal.org/files/issues/0001-Issue-1112462-YouTube-videos-no-longer-resize-to-fit.patch"
;
;    Can't delete YouTube media from library after Drupal 7.6 update
;      - AB 08/14/2011: http://drupal.org/node/1235852#comment-4858536
;      @see projects[media_youtube][patch][] = "http://drupal.org/files/issues/media_youtube-file-not-deleted-1235852-7.patch"
;
;    Allow users to reupload YouTube videos
;      - KS 10/10/2011: http://drupal.org/node/1213184#comment-4710568 and http://drupal.org/node/1213184#comment-5100580
;      @see projects[media_youtube][patch][] = "http://drupal.org/files/issues/1213184-media-youtube-allow-file-reuse.patch"
;      @see projects[media_youtube][patch][] = "http://drupal.org/files/1213184-4.reuse-existing.patch"
;
;    settings.options is not always set.
;      - JB 12/02/2011: http://drupal.org/node/1359684. This is fixed in HEAD. We just need to update.
;      - MS 12/28/2011: No patch available at #1359684. Diff still part of the combined patch listed above
;
;    wmode is not set on embeds, which means YouTube videos overlap dialogs in Chrome
;      - KS 2/23/2012: Issue was fixed in HEAD at http://drupal.org/node/1107930
;      @see projects[media_youtube][patch][] = "https://raw.github.com/gist/1894731/6d7ec2f937fd97e2cd17b319e5564a88f80a841c/media_youtube-fix-embed-wmode.patch"
;
projects[media_youtube][version] = "1.0-alpha5"
projects[media_youtube][patch][] = "https://raw.github.com/gist/5d8c97a21cd40b23a5e1/e477b036c58b68d65b7f25a0e27c6900ec0076e8/media_youtube-combined-diff.patch"
projects[media_youtube][patch][] = "https://raw.github.com/gist/1894731/6d7ec2f937fd97e2cd17b319e5564a88f80a841c/media_youtube-fix-embed-wmode.patch"

; Contrib module: modulefield
; ---------------------------------------
projects[modulefield][version] = "1.0-rc1"

; Contrib module: metatag
; ---------------------------------------
;
; Current patches:
;
;    Move tokens to a fieldset on metatags form
;      - AM 10/31/2011: http://drupal.org/node/1327694#comment-5185350
;      @see projects[metatag][patch][] = "http://drupal.org/files/tokens_to_fieldset_1327694_1.patch"
;
;    Improve text description of fields
;      - AM 11/01/2011: http://drupal.org/node/1328562#comment-5189724
;      @see projects[metatag][patch][] = "http://drupal.org/files/improve_field_descriptions_1328562_2.patch"
;
;    Combined patch (and reroll lists):
;      - MS 12/29/2011 https://gist.github.com/raw/e035224e12fca365d71a/ca3bdbd83e308ffe3524074a0d1585104de3193c/metatags_combined-diff.patch
;      - EG 09/30/2012 https://raw.github.com/gist/ce92bd63ff70e53ae95a/2ad476bcd60059f68a86017ad3a6ebdae5e877c8/new-metatags_combined-diff.patch
;      - BN 12/07/2012 https://raw.github.com/gist/8769b20c2b902b38b8b0/ad17c00dba1d9a7feebed99b83b8c0315ef704c3/metatag_10beta2_acquia_changes_rerolled.patch
;
projects[metatag][version] = "1.0-beta2"
projects[metatag][patch][] = "https://raw.github.com/gist/8769b20c2b902b38b8b0/ad17c00dba1d9a7feebed99b83b8c0315ef704c3/metatag_10beta2_acquia_changes_rerolled.patch"

; Contrib module: migrate
; ---------------------------------------
projects[migrate][type] = "module"
projects[migrate][download][type] = "git"
projects[migrate][download][url] = "http://git.drupal.org/project/migrate.git"
projects[migrate][download][revision] = "1e29781bcbd9140d1ba3e8ae3f1288117670598b"

; Contrib module: migrate_extras
; ---------------------------------------
projects[migrate_extras][type] = "module"
projects[migrate_extras][download][type] = "git"
projects[migrate_extras][download][url] = "http://git.drupal.org/project/migrate_extras.git"
projects[migrate_extras][download][revision] = "f42c6259cc6490fc4b7bc8266164218ace43573e"

; Contrib module: migrate_extras
; ---------------------------------------
projects[migrate_extras][type] = "module"
projects[migrate_extras][download][type] = "git"
projects[migrate_extras][download][url] = "http://git.drupal.org/project/migrate_extras.git"
projects[migrate_extras][download][revision] = "f42c6259cc6490fc4b7bc8266164218ace43573e"

; Contrib module: mollom
; ---------------------------------------
projects[mollom][version] = "2.4"

; Contrib module: multiform
; ---------------------------------------
projects[multiform][version] = "1.0"

; Contrib module: node_export
; ---------------------------------------
;
; The git commit we are using is actually the 3.0 tag, we are pulling it from git because
; there are patches affecting the .info file.

projects[node_export][type] = "module"
projects[node_export][download][type] = "git"
projects[node_export][download][url] = "http://git.drupal.org/project/node_export.git"
projects[node_export][download][revision] = "ebef56784374f977f3cfdb87f4a5ba42182b3477"
projects[node_export][patch][] = "http://drupal.org/files/node-export-xml-import-export-1717298-1.patch"
projects[node_export][patch][] = "http://drupal.org/files/node-export-xml-import-empty-array-1727872-1.patch"
projects[node_export][patch][] = "http://drupal.org/files/node-export-alter-format-handlers-1742614-1.patch"

; Contrib module: oauth
; ---------------------------------------
projects[oauth][version] = "3.0"
projects[oauth][patch][] = http://drupal.org/files/1404030-1.oauth-notices.patch
; Patch to make oauth login path configurable
projects[oauth][patch][] = http://drupal.org/files/1710752-1.patch
; Patch to recognize Authorization header under FastCGI
projects[oauth][patch][] = http://drupal.org/files/1365168-4.fastcgi-fix.patch

; Contrib module: oembed
; ---------------------------------------
;
; Current patches:
;
;  Combined patch for all issues below:
;    MS 12/29/11
;    @see projects[oembed][patch][] = "https://raw.github.com/gist/2375351/175b730b9889772415d771df5f51fe5f955d1aa8/oembed_combined-diff.2.patch"
;
;    Allow media_oembed to use the media_embed resizing library that is being developed for the media module
;      - JE 09/13/2011: http://drupal.org/node/1278306#comment-4984688
;        - AB 10/03/2011: Updated patch: http://drupal.org/node/1278306#comment-5069710.
;      @see projects[oembed][patch][] = "http://drupal.org/files/oembed-resize_0.patch"
;
;    oembedcore_oembed_fetch fails silently when the URL it fetches returns an HTTP error
;      - JB 10/03/2011: http://drupal.org/node/1298210
;        - MS 12/01/2011: Updated patch for new tag: http://drupal.org/node/1298210#comment-5313264
;        - DR 03/27/2012: Updated patch to fix a bug: http://drupal.org/node/1298210#comment-5791154
;      @see projects[oembed][patch][] = "http://drupal.org/files/fetch-of-bad-URL-fails-silently_1298210_5-BETA2.patch"
;
;    Entering URL of an asset already in {file_managed} leads to PDO exception
;      - AB 10/03/2011: http://drupal.org/node/1298562
;        - KS 10/7/2011: Updated patch: http://drupal.org/node/1298562#comment-5089160
;      @see projects[oembed][patch][] = "http://drupal.org/files/1298562-1.oembed-duplicate-uri.patch"
;
;    Allow Media Browser Plus to autopopulate fields from oEmbed data
;      - AB 10/03/2011: http://drupal.org/node/1298566
;      @see projects[oembed][patch][] = "http://drupal.org/files/oembed-media-browser-plus.patch"
;
;    Don't display link text above embedded media items
;      - KS 10/4/2011: http://drupal.org/node/1299714#comment-5075482
;      @see projects[oembed][patch][] = "http://drupal.org/files/1299714-1.media_oembed-omit-link.patch"
;
;    Fix a couple things related to thumbnails
;      - AB 10/10/2011: http://drupal.org/node/1305650
;      @see projects[oembed][patch][] = "http://drupal.org/files/oembed-thumbnail.patch"
;
;    Make media_oembed_cache_clear() more thorough
;      - AB 12/21/2011: http://drupal.org/node/1294824#comment-5392376
;      @see projects[oembed][patch][] = "http://drupal.org/files/oembed-media_cache_clear.patch"
;
;    Allow configuration of Flash wmode parameter
;      - AB 12/23/2011: http://drupal.org/node/1289202#comment-5399412
;
;    Add ability to tweak oembed request cache parameters
;      - AE 04/13/2012: http://drupal.org/node/1530232
;      @see projects[oembed][patch][] = "http://drupal.org/files/oembed_cache_alter.patch"
;
projects[oembed][version] = "0.1-beta2"
projects[oembed][patch][] = "https://raw.github.com/gist/2375351/175b730b9889772415d771df5f51fe5f955d1aa8/oembed_combined-diff.2.patch"

; Contrib module: options_element
; ---------------------------------------
;
; Current patches:
;
;  Combined patch for all issues below:
;    MS 12/29/11
;    @see projects[options_element][patch][] = "https://raw.github.com/gist/493b36618b7560609999/530365f83cc545f6dd37416bacd571ac24fed192/options_element-combined-diff.patch"
;
;    Move the organizational / functional alterations out of theme_options and into a pre_render function
;      - JE 12/28/2010: http://drupal.org/node/1009104
;        - AB 02/03/2011: Updated patch to reflect actual code in trunk: http://drupal.org/node/1009104#comment-4044628
;      @see projects[options_element][patch][] = "http://drupal.org/files/issues/options_element_pre_render_0.patch"
;
;    Elements with #default_value of FALSE can't have their default value edited again (breaks Webform integration)
;      - DR 6/23/2011: http://drupal.org/node/1198142 (first patch)
;      @see projects[options_element][patch][] = "http://drupal.org/files/issues/options-element-manual-default-value.patch"
;
projects[options_element][version] = "1.4"
projects[options_element][patch][] = "https://raw.github.com/gist/493b36618b7560609999/530365f83cc545f6dd37416bacd571ac24fed192/options_element-combined-diff.patch"

;
; Contrib module: original_author
; ---------------------------------------
projects[original_author][version] = "1.0-rc1"

;
; Contrib module: pathauto
; ---------------------------------------
projects[pathauto][version] = "1.2"
projects[pathauto][patch][] = "http://drupal.org/files/issues/936222-pathauto-persist-16.patch"
projects[pathauto][patch][] = "http://drupal.org/files/1565850-hook-pathauto-alias-alter.patch"
; Fix for test failures resulting from 936222 that cannot be added to d.o due to issue divergence
projects[pathauto][patch][] = "https://raw.github.com/gist/f0cfe6cd6464004ee30f/332910338311039a94b15ce0f10b54dcf5c96aa7/DG-1665_pathauto_test.patch"

; Contrib module: plupload
; ---------------------------------------
;   Improper file API validation
;   - KH 10/16/2012 http://drupal.org/node/1814744#comment-6611148
;   @see http://drupal.org/files/plupload_hook_validate_uri.patch
;   - KH 10/30/2012 http://drupal.org/node/1827368
;   @see http://drupal.org/files/plupload_file_validate_message.patch
;
projects[plupload][version] = "1.0"
projects[plupload][patch][] = "http://drupal.org/files/plupload_hook_validate_uri.patch"
projects[plupload][patch][] = "http://drupal.org/files/plupload_file_validate_message.patch"

; Contrib module: redirect
; ---------------------------------------
;   Multiple entities pointing at the same alias.
;   - BN 11/30/2012 http://drupal.org/node/1288768#comment-6796086
projects[redirect][version] = "1.0-rc1"
projects[redirect][patch][] = "http://drupal.org/files/migrate_redirect-1116408-39.patch"
projects[redirect][patch][] = "http://drupal.org/files/redirect-detect_prevent_circular_redirects_patch_and_test-1796596-18.patch"
projects[redirect][patch][] = "http://drupal.org/files/redirect_maintenance_on_new_path-1288768-2.patch"

; Contrib module: references
; ---------------------------------------
projects[references][version] = "2.0-beta3"

; Contrib module: request_queue
; ---------------------------------------
projects[request_queue][type] = "module"
projects[request_queue][download][type] = "git"
projects[request_queue][download][url] = "http://git.drupal.org/project/request_queue.git"
projects[request_queue][download][revision] = "d30366d3d84550e843913f6f606635a54bd930f8"

; Contrib module: remote_stream_wrapper
;
;   7.x-1.x at commit fdaff422adc55fc01ba94974d5743feeeb45b3f7
;
; ---------------------------------------
projects[remote_stream_wrapper][version] = "1.0-beta4"
; http://drupal.org/node/1299438#comment-5075172
projects[remote_stream_wrapper][patch][] = "http://drupal.org/files/remote_stream_wrapper-image-derivatives_1.patch"
; See https://github.com/acquia/gardens/commit/8570f732e1524b2e98741ef6d259525ec6f26a0e
projects[remote_stream_wrapper][patch][] = "https://raw.github.com/gist/9436661f378c7eca6253/7c117ef87bed78a0bca97169c561b240b7354696/catch_recursive_calls_to_getmimetype.patch"

; Contrib module: rotating_banner
; ---------------------------------------
projects[rotating_banner][version] = "1.x-dev"
projects[rotating_banner][patch][] = "http://drupal.org/files/issues/1160786-1.file_entity.patch"
projects[rotating_banner][patch][] = "http://drupal.org/files/rotating_banner-no-block-caching-965840.patch"

; Contrib module: save_draft
; ---------------------------------------
projects[save_draft][type] = "module"
projects[save_draft][download][type] = "git"
projects[save_draft][download][url] = "http://git.drupal.org/project/save_draft.git"
projects[save_draft][download][revision] = "7be034381e0e8e64727ad62b6356f3ebfd8e1ca1"
projects[save_draft][patch][] = "http://drupal.org/files/save-as-draft-submit-handler-1446730-2.patch"

; Contrib module: securepages
;
;  master branch at commit 9c282ec60e43fd78ceb749114e68c561efbf3447
;
; ---------------------------------------
projects[securepages][type] = "module"
projects[securepages][download][type] = "git"
projects[securepages][download][url] = "http://git.drupal.org/project/securepages.git"
projects[securepages][download][revision] = "9c282ec60e43fd78ceb749114e68c561efbf3447"
projects[securepages][patch][] = "http://drupal.org/files/1850136-securepages-alter.patch"

; Contrib module: seo_ui
; ---------------------------------------
projects[seo_ui][version] = "1.0"
; See http://drupal.org/node/1394342#comment-6439188, undefined indicies
projects[seo_ui][patch][] = "http://drupal.org/files/1394342_fix-undefined-indices_15.patch"
; FIx seo_ui to find metatag submit function and avoid a fatal error.
projects[seo_ui][patch][] = "http://drupal.org/files/seo_ui_doesnt_find_meatatag_submit_function-1461232-10.patch"

; Contrib module: server_variables
; ---------------------------------------
projects[server_variables][type] = "module"
projects[server_variables][download][type] = "git"
projects[server_variables][download][url] = "http://git.drupal.org/sandbox/drupalgardens/1389764.git"
projects[server_variables][download][revision] = "4850a54712dac6407218773dd06a13a4c6193e6e"
; See https://github.com/acquia/gardens/commit/f391eb5b3384fcf2ee762a29e30690f4d177f2c7
projects[server_variables][patch][] = "https://raw.github.com/gist/9292739e55129e99d505/174f3c0b5f86f19ef87277c30748e06fc5d62962/DG-3179_server_variables.patch"

; Contrib module: services
; ---------------------------------------
projects[services][version] = "3.3"

; Contrib module: simplified_menu_admin
; ---------------------------------------
projects[simplified_menu_admin][version] = "1.0-beta2"

; Contrib module: simpleviews
; ---------------------------------------
;
;   As of 12/30/11 the version of simpleviews in the Gardens codebase
;   is a CVS checkout with a few patches applied, so here we're pulling it from Acquia's SVN repo,
;   because it's easy.
;
;   TODO
;   - Convert to a proper Git checkout from d.o
;   - Apply patch(es) if they're still relevant, looks like Gabor and Katherine
;     have some on d.o. It's not clear though from PATCHES.txt which are actually
;     being applied right now. Here's the note from PATCHES.txt:
;
;       Acquia upgrade of SimpleViews to Drupal 7, removing the Views dependency: http://drupal.org/node/615882#comment-3998036
;
projects[simpleviews][type] = module
projects[simpleviews][download][type] = svn
projects[simpleviews][download][url] = https://svn.acquia.com/repos/engineering/gardens/trunk/docroot/sites/all/modules/simpleviews/

; Contrib module: simplified_modules
; ---------------------------------------
projects[simplified_modules][version] = "1.0-beta1"

; Contrib module: site_verify
; ---------------------------------------
projects[site_verify][version] = "1.0"

; Contrib module: shield
; ---------------------------------------
projects[shield][version] = "1.2"
projects[shield][patch][] = "http://drupal.org/files/1706902.display_401_unauthorized.patch"

; Contrib module: statsd
; ---------------------------------------
projects[statsd][version] = "1.0-beta1"

; Contrib module: styles
; ---------------------------------------
;
; Note from PATCHES.txt:
;
;   Styles (Irrelevant: disabled on all sites. TODO: uninstall and remove from codebase)
;   - Some, but irrelevant
;
projects[styles][type] = module
projects[styles][download][type] = svn
projects[styles][download][url] = https://svn.acquia.com/repos/engineering/gardens/trunk/docroot/sites/all/modules/styles/

; Contrib module: sort_comments
;
;   At commit 8618531420aa88c4dd196faf0f2a9c74ae853378
;
; ---------------------------------------
;
; Notes:
;
;  - JB 12/15/11: I've contacted the maintainer about releasing a tag for this module.
;
;projects[sort_comments][type] = "module"
;projects[sort_comments][download][type] = "git"
;projects[sort_comments][download][url] = "http://git.drupal.org/project/sort_comments.git"
;projects[sort_comments][download][revision] = "8618531420aa88c4dd196faf0f2a9c74ae853378"

; Contrib module: tac_alt_ui (sandbox project 1363014)
; ---------------------------------------
projects[tac_alt_ui][type] = "module"
projects[tac_alt_ui][download][type] = "git"
projects[tac_alt_ui][download][url] = "http://git.drupal.org/sandbox/pwolanin/1363014.git"
projects[tac_alt_ui][download][revision] = "eae8fa8dae213d12140262fd1dae5d5aba5dafeb"

; Contrib module: taxonomy_access
;
;  7.x-1.x at commit 9b6d358c420dced96e15d537111fc09fcf07881f
;
; ---------------------------------------
projects[taxonomy_access][type] = "module"
projects[taxonomy_access][download][type] = "git"
projects[taxonomy_access][download][url] = "http://git.drupal.org/project/taxonomy_access.git"
projects[taxonomy_access][download][revision] = "9b6d358c420dced96e15d537111fc09fcf07881f"
projects[taxonomy_access][patch][] = "http://drupal.org/files/1399260-4.admin-role-defaults.patch"
projects[taxonomy_access][patch][] = "http://drupal.org/files/1438232.tac-views.patch"
; KB 4/29/2012: probably a result of a file being left out of the above patch, but I rolled a gist to add it in.
projects[taxonomy_access][patch][] = "https://raw.github.com/gist/29571806ccd214216ef1/20d3f7c04550bd35e09a26d52a51c3228f06f72b/DG-2801-tac_views.patch"

; Contrib module: timeago
; ---------------------------------------
;
; Current patches:
;
;  Libraries 1.0 is not supported, fallback should take this into account
;    - KG 10/08/12: http://drupal.org/node/1781134#comment-6572720
;    @see projects[timeago][patch][] = "http://drupal.org/files/timeago-libraries-support-1781134-5.patch"
;
projects[timeago][version] = "2.1"
projects[timeago][patch][] = "http://drupal.org/files/timeago-libraries-support-1781134-5.patch"
projects[timeago][patch][] = "http://drupal.org/files/timeago-javascript_errors-1886166-1.patch"
;  Fix Javascript load order with Libraries 2.0.
projects[timeago][patch][] = "http://drupal.org/files/timeago-wrong-file-include-order-1832550-4.patch"

; Contrib module: token
; ---------------------------------------
;
; Current patches:
;
;  Make token titles that have child-tokens clickable
;    - JSW 11/01/2011: http://drupal.org/node/1328546#comment-5189110
;      - MS 12/27/2011: Re-roll for drush make
;    @see projects[token][patch][] = "http://drupal.org/files/clickable_token_titles_1328546_7.patch"
;
;  Fix token table indent bug.
;    - CB 11/01/2011 http://drupal.org/node/961130#comment-5188756
;    @see projects[token][patch][] = "http://drupal.org/files/token_indent-961130-11.patch"
;
;  - KB 04/29/2012: Rolled changes that were missing from the make file as a gist:
;  Original commit: https://github.com/acquia/gardens/commit/6b3bbdddef6fd4b59a05819ed09b993a74de6425
;
projects[token][version] = "1.2"
projects[token][patch][] = "http://drupal.org/files/clickable_token_titles_1328546_7.patch"
projects[token][patch][] = "http://drupal.org/files/token_indent-961130-11.patch"
; See https://github.com/acquia/gardens/commit/6b3bbdddef6fd4b59a05819ed09b993a74de6425
projects[token][patch][] = "https://raw.github.com/gist/b82909a4af6b1b679bcb/e8662d6a95b37199558bc96b6978279aaa753565/DG-424-410-token_tree_css.patch"

; Contrib module: token_filter
; ---------------------------------------
projects[token_filter][version] = "1.1"

; Contrib module: typekit
; ---------------------------------------
projects[typekit][version] = "1.0-beta1"

; Contrib module: uuid
; ---------------------------------------
projects[uuid][version] = "1.0-alpha3"

; Contrib module: ux_elements
; ---------------------------------------
projects[ux_elements][version] = "1.0-beta1"

; Contrib module: views
; ---------------------------------------
projects[views][version] = "3.1"
; http://drupal.org/node/1249868 (rerolled as gist due to problem with views_rss)
projects[views][patch][] = "https://raw.github.com/gist/75bee1e7742ce26f33de/fa7184967765130b41a2180f0e35e2030c8e3383/1249868.views-formatter-settings-inline.patch"
projects[views][patch][] = "http://drupal.org/files/1416018-1.sql-rewrite-warning.patch"
; http://drupal.org/node/1096648#comment-5573426
projects[views][patch][] = "http://drupal.org/files/views-drush-expimp.patch"
; http://drupal.org/node/1443244
projects[views][patch][] = "http://drupal.org/files/views-last-comment-uid-broken.patch"
projects[views][patch][] = "http://drupal.org/files/views_menu_rebuild_cache_clear-1280382-19.patch"
; http://drupal.org/node/1461236
projects[views][patch][] = "http://drupal.org/files/views_menu_rebuild.patch"
; http://drupal.org/node/1482824#comment-5733720
projects[views][patch][] = "http://drupal.org/files/views-ajax-pager.patch"
; http://drupal.org/node/1807916#comment-6711528 - fix redirect loop on "reset" on exposed forms
projects[views][patch][] = "http://drupal.org/files/views-exposed-form-reset-redirect-1807916-4.patch"

; Contrib module: views_bulk_operations
;
; ---------------------------------------
projects[views_bulk_operations][version] = "3.1"
; Prevent PHP code execution when VBO is run on Gardens servers
projects[views_bulk_operations][patch][] = "https://gist.github.com/katbailey/3fefba13cb1b25f7d44d/raw/3c5f24bae514924ca09d31a63bd058897650ecd7/vbo-script-action.patch"

; Contrib module: views_data_export
;
; ---------------------------------------
projects[views_data_export][version] = "3.0-beta6"
; http://drupal.org/node/1782038 - make the temp file directory configurable
projects[views_data_export][patch][] = "http://drupal.org/files/views_data_export_dir_conf.patch"

; Contrib module: views_load_more
; ---------------------------------------
;
; Current patches:
;
;  Combined patch for all issues below:
;    @see projects[views_load_more][] = "https://raw.github.com/gist/2001048/37ede839137c4f200c4b29e8d396a88e654d4880/views_load_more-combined-diff3.patch"
;
;    Don't allow 'unlimited' number of items per page to be valid
;      - AM 10/24/11: http://drupal.org/node/1319830#comment-5155762
;      @see projects[views_load_more][patch][] = "http://drupal.org/files/items-per-page-validation-1319830-2.patch"
;
;    Provide more descrptive description for the module
;      - AM 10/24/11: http://drupal.org/node/1320164#comment-5156700
;      @see projects[views_load_more][patch][] = "http://drupal.org/files/provide_more_descriptive_description_1320164_1.patch"
;
;    Allow admin to change the text of load more
;      - AM 10/28/11: http://drupal.org/node/1272562#comment-5193072
;      @see projects[views_load_more][patch][] = "http://drupal.org/files/change_text_per_view_1272562_7.patch"
;
;    (see r37812) Properly place newly returned values in list-style views
;      - KS 12/15/11: http://drupal.org/node/1372206#comment-5369488
;      @see projects[views_load_more][patch][] = "http://drupal.org/files/loaded_outside_of_list_1372206_1.patch"
;
;    Handle latest views 3 update (which removed the views-processed class after attaching ajax behaviors)
;      - AE 3/1/12: Cherry-pick the fix from http://drupal.org/node/1439778
;      @see projects[views_load_more][patch][] = "https://raw.github.com/gist/1949199/8cdf396ab1575d2fde48d4feabba3fb6084777bc/gistfile1.diff"
;
;    The latest views update also introduced a bug where views_load_more pagers would result in the content being scrolled to the top.
;      - KB 3/7/12: Applied the fix from http://drupal.org/node/1404664#comment-5508872 - NOT included in the diff patch below
;      - AE 3/8/12: Added the patch to our combined diff, below.  Should be all set.
;      @see projects[views_load_more][patch][] = "http://drupal.org/files/remove_scroll_to_top-no_space.patch"
;  Avoid to declare viewsLoadMoreAppend function if Drupal.Ajax is not available. Error with JavaScript aggregation.
;    - AM 9/6/12: http://drupal.org/node/1703436#comment-6444238
;    @see projects[views_load_more][patch][] = "https://raw.github.com/gist/206a4d829bdcc4c67fb3/0764a244aafa93858b9b7a9da35bad8c40c206cc/views_load_more-avoid_error_undefined_prototype_property_with_js_aggregation-1703436-5.diff"
;
projects[views_load_more][version] = "1.1"
projects[views_load_more][patch][] = "https://raw.github.com/gist/2001048/37ede839137c4f200c4b29e8d396a88e654d4880/views_load_more-combined-diff3.patch"
projects[views_load_more][patch][] = "https://raw.github.com/gist/206a4d829bdcc4c67fb3/0764a244aafa93858b9b7a9da35bad8c40c206cc/views_load_more-avoid_error_undefined_prototype_property_with_js_aggregation-1703436-5.diff"

; Contrib module: views_rss
;
;  7.x-2.x at commit 69321d0284f8417a51e463ed4bd0fcd5a3e514e7 which includes
;  support for media module.
;
; ---------------------------------------
projects[views_rss][type] = "module"
projects[views_rss][download][type] = "git"
projects[views_rss][download][url] = "http://git.drupal.org/project/views_rss.git"
projects[views_rss][download][revision] = "69321d0284f8417a51e463ed4bd0fcd5a3e514e7"

; Contrib module: views_rss_itunes
; ---------------------------------------
projects[views_rss_itunes][type] = "module"
projects[views_rss_itunes][download][type] = "git"
projects[views_rss_itunes][download][url] = "http://git.drupal.org/project/views_rss_itunes.git"
projects[views_rss_itunes][download][revision] = "99c99635bcc7df494797ee0773d0727b2021b9d1"
projects[views_rss_itunes][patch][] = "http://drupal.org/files/1846076.views_rss_itunes.requirements.patch"

; Contrib module: votingapi
; ---------------------------------------
projects[votingapi][version] = "2.4"
projects[votingapi][patch][] = "http://drupal.org/files/issues/1227002-1-remove-votingapi-variables-on-uninstall.patch"
projects[votingapi][patch][] = "http://drupal.org/files/issues/997092-20.votingapi-anonymous-votes.patch"
; http://drupal.org/node/1069942#comment-4150616
projects[votingapi][patch][] = "http://drupal.org/files/issues/votingapi_cron_db_error.patch"

; Contrib module: webform
; ---------------------------------------
;
; Current patches:
;
;  Allow additional filters to be passed to the submission results page
;    - JE 2/21/11: http://drupal.org/node/1068294
;      - AB 3/22/11: Updated patch: http://drupal.org/node/1068294#comment-4247354
;      - AB 5/17/11: Updated patch: http://drupal.org/node/1068294#comment-4482202
;      - MS 11/3/11: Updated patch: http://drupal.org/node/1068294#comment-5198140
;      - AB 12/28/11: Updated patch: http://drupal.org/node/1068294#comment-5409738
;      - DR 1/24/12: Updated patch: http://drupal.org/node/1068294#comment-5510750
;    @see projects[webform][patch][] = "http://drupal.org/files/webform-submission_filters-1068294-10.patch"
;
;  Webform contextual links are shown to users who don't have access to them, and code can be simplified
;    - DR 1/23/2012: http://drupal.org/node/1414666
;    @see projects[webform][patch][] = "http://drupal.org/files/webform-contextual-links.patch"
;
projects[webform][version] = "3.15"
projects[webform][patch][] = "http://drupal.org/files/webform-submission_filters-1068294-10.patch"
projects[webform][patch][] = "http://drupal.org/files/webform-contextual-links.patch"

; Contrib module: webform_alt_ui
; ---------------------------------------
;
; Current patches:
;
;  Add support for extra filtering of webform submissions
;    - AB 5/17/2011: http://drupal.org/node/1160850
;      - DR 6/20/2011: We should wait to commit this until http://drupal.org/node/1068294 (listed above under the Webform module) is committed.
;    @see projects[webform_alt_ui][patch][] = "http://drupal.org/files/webform_alt_ui-submission_filters_REROLL.patch"
;
;  Reveal the default email settings for webforms that webform_alt_ui had previously hidden through #access restriction.
;    - JB 5/23/2012: http://drupal.org/node/1597398
;    @see projects[webform_alt_ui][patch][] = "http://drupal.org/files/1597398_reveal-default-email-configs_1.patch"
;
projects[webform_alt_ui][version] = "1.0-alpha5"
projects[webform_alt_ui][patch][] = "http://drupal.org/files/webform_alt_ui-submission_filters_REROLL.patch"
projects[webform_alt_ui][patch][] = "http://drupal.org/files/1597398_reveal-default-email-configs_1.patch"

; Contrib module: webform_ssl
; ---------------------------------------
projects[webform_ssl][version] = "1.0-beta1"

; Contrib module: wordpress_migrate
; ---------------------------------------
projects[wordpress_migrate][type] = "module"
projects[wordpress_migrate][download][type] = "git"
projects[wordpress_migrate][download][url] = "http://git.drupal.org/project/wordpress_migrate.git"
projects[wordpress_migrate][download][revision] = "e85ac0ad799e0e2f89c54d2bc3f610f4f430b7c3"
projects[wordpress_migrate][patch][] = "http://drupal.org/files/1826536-1.deletion-message.patch"
projects[wordpress_migrate][patch][] = "http://drupal.org/files/1828708-1.ui-text.patch"

; Contrib module: wysiwyg
; ---------------------------------------
projects[wysiwyg][version] = "2.1"
; http://drupal.org/node/1155678
projects[wysiwyg][patch][] = "http://drupal.org/files/wysiwyg.detach.13.patch"
projects[wysiwyg][patch][] = "http://drupal.org/files/507696-65.wysiwyg_per_field.patch"
projects[wysiwyg][patch][] = "http://drupal.org/files/1388224-1.wysiwyg-detach-serialize.patch"

; Contrib module: xmlsitemap
; ---------------------------------------
projects[xmlsitemap][version] = "2.0-rc1"
projects[xmlsitemap][patch][] = "http://drupal.org/files/issues/xmlsitemap-809250-2.patch"
projects[xmlsitemap][patch][] = "http://drupal.org/files/issues/1008566_revert.patch"


; Contrib module: imagemagick
; ---------------------------------------
projects[imagemagick][version] = "1.0-alpha2"

; Library: breakup
; Version: 1.0
; ---------------------------------------
libraries[breakup][destination] = "libraries"
libraries[breakup][download][type] = "git"
libraries[breakup][download][url] = "https://github.com/jessebeach/breakup.git"
libraries[breakup][download][revision] = "b58a305f1cd4e7b5670d91e7d25c4fd1a64aab05"

; Contrib module: session_cookie_lifetime
projects[session_cookie_lifetime][version] = "1.2"

; Library: ckeditor
; Version: 3.5.1
; ---------------------------------------
libraries[ckeditor][destination] = "libraries"
libraries[ckeditor][download][type] = "get"
libraries[ckeditor][download][url] = "http://download.cksource.com/CKEditor/CKEditor/CKEditor%203.5.1/ckeditor_3.5.1.tar.gz"
libraries[ckeditor][directory] = "ckeditor"

; Library: fancybox
; Version: 1.3.4
; ---------------------------------------
libraries[fancybox][destination] = "libraries"
libraries[fancybox][download][type] = "get"
libraries[fancybox][download][url] = http://fancybox.googlecode.com/files/jquery.fancybox-1.3.4.zip

; Library: getID3()
; Version: 1.9.3
; ---------------------------------------
libraries[getid3][destination] = "libraries"
libraries[getid3][download][type] = "get"
libraries[getid3][download][url] = http://sourceforge.net/projects/getid3/files/getID3%28%29%201.x/1.9.3/getid3-1.9.3-20111213.zip/download

; Library: jquery.imgareaselect
; Version: 0.9.8
; ---------------------------------------
libraries[jquery.imgareaselect][destination] = "libraries"
libraries[jquery.imgareaselect][download][type] = "get"
libraries[jquery.imgareaselect][download][url] = "http://odyniec.net/projects/imgareaselect/jquery.imgareaselect-0.9.8.zip"
libraries[jquery.imgareaselect][directory] = "jquery.imgareaselect"

; Library: htmlpurifier
; Version: 4.4.0
; ---------------------------------------
libraries[htmlpurifier][destination] = "libraries"
libraries[htmlpurifier][download][type] = "get"
libraries[htmlpurifier][download][url] = "http://htmlpurifier.org/releases/htmlpurifier-4.4.0-lite.zip"
libraries[htmlpurifier][directory] = "htmlpurifier"

; Library: colorbox
; Version: 953bc129e765ddadfb3dd170a85ee41c16405095
; ---------------------------------------
;
libraries[colorbox][destination] = "libraries"
libraries[colorbox][download][type] = "git"
libraries[colorbox][download][url] = "https://github.com/jackmoore/colorbox.git"
libraries[colorbox][download][revision] = "953bc129e765ddadfb3dd170a85ee41c16405095"

; Library: plupload
; Version: 1.4.3.2
; ---------------------------------------
;
; Needs review:
;
;  We removed the examples and src directories from this library, apparently for security reasons
;    - (see AN-18571 and https://svn.acquia.com/fisheye/changelog/Engineering?cs=18379)
;    - MS 12/28/2011: Need a patch URL so drush make can download it
;
libraries[plupload][destination] = "libraries"
libraries[plupload][download][type] = "get"
libraries[plupload][download][url] = "https://github.com/downloads/moxiecode/plupload/plupload_1_5_1_1.zip"

; Library: timeago
; ---------------------------------------
libraries[timeago][destination] = "libraries"
libraries[timeago][download][type] = "get"
libraries[timeago][download][url] = "http://timeago.yarp.com/jquery.timeago.js"
libraries[timeago][directory] = "timeago"

; Library: wvega.timepicker
; Version: master
; ---------------------------------------
;
; Needs review:
;
;  Which branch/tag/revision should we be using?
;
libraries[wvega-timepicker][destination] = "libraries"
libraries[wvega-timepicker][download][type] = "git"
libraries[wvega-timepicker][download][url] = "https://github.com/wvega/timepicker.git"
libraries[wvega-timepicker][download][revision] = "8228c806ea87de80a8391657f58a51d2487ad8ec"
libraries[wvega-timepicker][directory_name] = "wvega-timepicker"

; Library: jQuery Form Plugin
; Version: 2.52-patched
; ---------------------------------------
;
; Our version is currently in sites/all/modules/gardens_features/plugins/jquery.form.js,
; which overrides the version shipped with Drupal core.
;
; Patched for Drupal Gardens (version 2.52-patched):
; - Add a hasOwnProperty check to prevent overridden array methods from being
;   processed as array elements during AJAX form submits. This fixes a bug
;   that affects the Janrain widget on IE8.
;   (https://svn.acquia.com/fisheye/changelog/Engineering/?cs=41606)
; - @todo: The patch is no longer needed when we update to the latest version
;   of the plugin, since the bug is fixed there also.
;
