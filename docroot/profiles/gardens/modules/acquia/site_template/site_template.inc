<?php
// $Id$

/**
 * @file
 * Definition of DrupalSiteTemplate.
 */

class DrupalSiteTemplate {
  /**
   * Site template callback for updating node types at the end of the install.
   */
  public function site_template_update_node_types_install(&$data, $features) {
    // If the comment feature is being used, make sure to force anonymous users
    // to provide contact info on all newly-added node types.
    if (in_array('site_template_comment_feature', $features)) {
      site_template_force_anonymous_contact();
    }

    // If the comment notify feature is being used, make sure to enable comment
    // notification on all newly-added node types.
    if (in_array('site_template_comment_notify_feature', $features)) {
      site_template_enable_comment_notification();
    }
  }

  /**
   * Site template preparation callback for adding a blogger role.
   */
  public function site_template_add_blogger_role_install_prepare(&$data, $features) {
    $data['role_name'] = 'blogger';
    // Bloggers can create and edit/delete their own blog posts and polls.
    $permissions = array();
    foreach (array('blog', 'poll') as $type) {
      $permissions[] = "create $type content";
      $permissions[] = "edit own $type content";
      $permissions[] = "delete own $type content";
    }
    $data['role_permissions'] = array_merge($this->baseContentCreatorPermissions(), $permissions);
  }

  /**
   * Site template callback for adding a blogger role.
   */
  public function site_template_add_blogger_role_install(&$data, $features) {
    // Only add the role if the blog feature is enabled.
    if (in_array('site_template_blog_feature', $features)) {
      site_template_add_basic_role($data['role_name'], $data['role_permissions']);
    }
  }

  /**
   * Site template preparation callback for adding an editor role.
   */
  public function site_template_add_editor_role_install_prepare(&$data, $features) {
    $data['role_name'] = 'editor';
    // Editors can create, edit and delete all content types.
    $permissions = array();
    foreach (array_keys(node_type_get_types()) as $type) {
      $permissions = array_merge($permissions, array_keys(node_list_permissions($type)));
    }
    $data['role_permissions'] = array_merge($this->baseContentCreatorPermissions(), $permissions);
  }

  /**
   * Site template callback for adding an editor role.
   */
  public function site_template_add_editor_role_install(&$data, $features) {
    site_template_add_basic_role($data['role_name'], $data['role_permissions']);
  }

  /**
   * Site template preparation callback for adding a sample article.
   */
  public function site_template_sample_article_install_prepare(&$data, $features) {
    // Prepare a sample article.
    $data['article'] = array(
      'type' => 'article',
      'title' => 'This is a sample news item',
      'body' => "<p>News items are intended for content such as announcements, press releases and other time-specific information. News items are one of many content types you can display on your site. This news item is promoted to the front page.</p><p>To change your site's front page, click Configuration > Site Information.<br />To add another News item, click Add content > News item.</p><a href='http://www.drupalgardens.com/learnmore/news' target='_blank'>Learn more</a>",
      'promote' => NODE_PROMOTED,
    );
  }

  /**
   * Site template preparation callback for adding an content manager role.
   */
  public function site_template_add_content_manager_role_install_prepare(&$data, $features) {
    $data['role_name'] = 'content manager';
    // Editors can create, edit and delete all content types.
    $node_permissions = array();
    foreach (array_keys(node_type_get_types()) as $type) {
      $node_permissions = array_merge($node_permissions, array_keys(node_list_permissions($type)));
    }
    $content_creator_permissions = $this->baseContentCreatorPermissions();
    $designer_permissions = $this->baseSiteDesignerPermissions();
    $data['role_permissions'] = array_unique(array_merge($node_permissions, $content_creator_permissions, $designer_permissions));
  }

  /**
   * Site template callback for adding a content manager role.
   */
  public function site_template_add_content_manager_role_install(&$data, $features) {
    site_template_add_basic_role($data['role_name'], $data['role_permissions']);
  }

  /**
   * Site template callback for adding a sample article.
   */
  public function site_template_sample_article_install(&$data, $features) {
    site_template_add_basic_node($data['article']);
  }

  /**
   * Rollback function for the feature that adds a sample article.
   */
  public function site_template_sample_article_rollback($data) {
    // TODO.
  }

  /**
   * Site template preparation callback for disabling the navigation block.
   */
  public function site_template_disable_navigation_block_install_prepare(&$data, $features) {
    // Store information about the navigation block which will be disabled.
    $theme = variable_get('theme_default', 'bartik');
    $data['navigation_block'] = array(
      'module' => 'system',
      'delta' => 'navigation',
      'theme' => $theme,
    );

    // Store the navigation block's current status now in case it will need to
    // be reenabled on rollback.
    $data['blocks_to_reenable'] = array();
    $original_navigation_block = db_query("SELECT * FROM {block} WHERE module = :module AND delta = :delta AND theme = :theme", array(':module' => 'system', ':delta' => 'navigation', ':theme' => $theme))->fetchAssoc();
    if (!empty($original_navigation_block['status'])) {
      $data['blocks_to_reenable'][] = $original_navigation_block;
    }
  }

  /**
   * Site template callback for disabling the navigation block.
   */
  public function site_template_disable_navigation_block_install(&$data, $features) {
    site_template_disable_block($data['navigation_block']);
  }

  /**
   * Rollback function for the feature that disables the navigation block.
   */
  public function site_template_disable_navigation_block_rollback($data) {
    foreach ($data['blocks_to_reenable'] as $block) {
      site_template_enable_block($block);
    }
  }

  /**
   * Site template preparation callback for adding a "Home" link.
   */
  public function site_template_add_home_link_install_prepare(&$data, $features) {
    $data['menu_link'] = array(
      'link_path' => '<front>',
      'link_title' => 'Home',
      'menu_name' => 'main-menu',
      'weight' => -28,
    );
  }

  /**
   * Site template callback for adding a "Home" link.
   */
  public function site_template_add_home_link_install(&$data, $features) {
    // Note: We are not using site_template_menu_link_save() here because the
    // Home link points to <front>, which does not have a router path.
    if (!empty($data['menu_link']) && ($mlid = menu_link_save($data['menu_link']))) {
      $data['mlid'] = $mlid;
    }
  }

  /**
   * Rollback function for the feature that adds a "Home" link.
   */
  public function site_template_add_home_link_rollback($data) {
    if (isset($data['mlid'])) {
      menu_link_delete($data['mlid']);
    }
  }

  /**
   * Site template preparation callback for the "Comment" feature.
   */
  public function site_template_comment_feature_install_prepare(&$data, $features) {
    // Define a block showing recent comments.
    $data['block_data'] = array(
      'module' => 'comment',
      'delta' => 'recent',
      'region' => 'sidebar_a',
      'title' => 'Comments',
    );
  }

  /**
   * Site template callback for the "Comment" feature.
   */
  public function site_template_comment_feature_install(&$data, $features) {
    // Allow users to post comments.
    $anonymous_permissions = array('access comments');
    $authenticated_permissions = array('access comments', 'post comments', 'skip comment approval');
    $data['permission_states'] = site_template_get_permission_states(array_unique(array_merge($anonymous_permissions, $authenticated_permissions)));
    user_role_grant_permissions(DRUPAL_ANONYMOUS_RID, $anonymous_permissions);
    user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, $authenticated_permissions);

    // Force anonymous users to provide their contact info. This is repeated
    // for new node types in site_template_update_node_types().
    $data['variable_states'] = site_template_force_anonymous_contact();

    // Enable the block that was prepared.
    $block = $data['block_data'];
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
  }

  /**
   * Rollback function for the "Comment" feature.
   */
  public function site_template_comment_feature_rollback($data) {
    site_template_restore_permissions_from_state($data['permission_states']);
    site_template_restore_variables_from_state($data['variable_states']);
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }
  }

  /**
   * Site template callback for the "Comment notify" feature.
   */
  public function site_template_comment_notify_feature_install(&$data, $features) {
    // Enable the module.
    module_enable(array('comment_notify'), FALSE);

    // Enable comment notification on all node types. This is repeated for new
    // node types in site_template_update_node_types().
    $data['variable_states'] = site_template_enable_comment_notification();

    // Allow anonymous and authenticated users to subscribe to comments.
    $permissions = array('subscribe to comments');
    $data['permission_states'] = site_template_get_permission_states($permissions);
    user_role_grant_permissions(DRUPAL_ANONYMOUS_RID, $permissions);
    user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, $permissions);
  }

  /**
   * Rollback function for the "Comment notify" feature.
   */
  public function site_template_comment_notify_feature_rollback($data) {
    site_template_restore_permissions_from_state($data['permission_states']);
    site_template_restore_variables_from_state($data['variable_states']);
    module_disable(array('comment_notify'), FALSE);
  }

  /**
   * Site template preparation callback for the "Follow us" feature.
   */
  public function site_template_follow_us_feature_install_prepare(&$data, $features) {
    // Define a block.
    $data['block_data'] = array(
      'module' => 'follow',
      'delta' => 'site',
      'region' => site_template_prefooter_regions(),
      'title' => 'Stay connected',
    );

    // Define links for the block.
    $link = new stdClass();
    $link->name = 'facebook';
    $link->url = 'http://www.facebook.com/pages/Drupal-Gardens/187669600960';
    $link->weight = -2;
    $data['links_to_save']['facebook'] = $link;
    $link = new stdClass();
    $link->name = 'twitter';
    $link->url = 'http://twitter.com/drupalgardens';
    $link->weight = -1;
    $data['links_to_save']['twitter'] = $link;
  }

  /**
   * Site template callback for the "Follow us" feature.
   */
  public function site_template_follow_us_feature_install(&$data, $features) {
    module_enable(array('follow'), FALSE);
    $block = $data['block_data'];
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
    $data['link_ids'] = array();
    foreach ($data['links_to_save'] as $link) {
      follow_link_save($link);
      $data['link_ids'][] = $link->lid;
    }
  }

  /**
   * Rollback function for the "Follow us" feature.
   */
  public function site_template_follow_us_feature_rollback($data) {
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }
    foreach ($data['link_ids'] as $link_id) {
      follow_link_delete($link_id);
    }
    module_disable(array('follow'), FALSE);
  }

  /**
   * Site template callback for the "Share this" feature.
   */
  public function site_template_share_this_feature_install(&$data, $features) {
    // Enable the addthis module.
    module_enable(array('addthis'), FALSE);

    // Rebuild the theme registry, because without this the "Share This" link
    // does not show up on sites where this is the only feature selected.
    // TODO: Figure out what horrible Drupal cache clearing bug makes this
    // necessary, solve that bug, and then remove this code.
    drupal_theme_rebuild();

    // Enable the "Share this" links.
    $variables = array('addthis_display_in_links', 'addthis_options');
    $data['variable_states'] = site_template_get_variable_states($variables);
    variable_set('addthis_display_in_links', TRUE);
    variable_set('addthis_options', 'twitter, facebook, favorites, email, digg, delicious, google, live, more');

    // Do not show the "Share this" link on pages by default, since that
    // usually gets in the way.
    variable_set('addthis_nodetype_page', 0);

    // Enable anonymous and authenticated users to view the "Share this" links.
    $permissions = array('view addthis');
    $data['permission_states'] = site_template_get_permission_states($permissions);
    user_role_grant_permissions(DRUPAL_ANONYMOUS_RID, $permissions);
    user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, $permissions);
  }

  /**
   * Rollback function for the "Share this" feature.
   */
  public function site_template_share_this_feature_rollback($data) {
    site_template_restore_permissions_from_state($data['permission_states']);
    site_template_restore_variables_from_state($data['variable_states']);
    module_disable(array('addthis'), FALSE);
  }

  /**
   * Site template preparation callback for the "Blog" feature.
   */
  public function site_template_blog_feature_install_prepare(&$data, $features) {
    // Prepare a couple of blog posts.
    $blog_post = array(
      'type' => 'blog',
      // We display blog posts from newest to oldest, so this is actually the
      // second post, not the first; hence the "another" in the title.
      'title' => 'This is another sample blog post',
      'body' => '<p>A blog post is a way to communicate your thoughts and ideas with associated images or videos. It is also a great way to engage your site visitors by allowing comments on your posts. Note that besides personal blogs, a single Drupal Gardens site can also host multiple blogs, each authored by a different user, which is a great way to give organizations a voice.</p><p>To add another Blog post, click Add content > Blog post.</p><p><a href="http://www.drupalgardens.com/learnmore/blog" target="_blank">Learn more</a></p>',
    );
    $data['blog_posts'][] = $blog_post;
    $blog_post['title'] = 'This is a sample blog post';
    $data['blog_posts'][] = $blog_post;

    // Prepare a comment to add to the last blog post.
    $data['comment'] = site_template_prepare_basic_comment('Sample comment', '<p>By default, comments are shown on blog posts, in a conversation thread. Anonymous or logged in visitors can post comments, and that can be configured.  Comments are protected from spam by the Mollom spam protection service.</p><p><a href="http://www.drupalgardens.com/learnmore/comments" target="_blank">Learn more</a></p>');

    // Prepare the recent comments block.
    $data['block_data'] = array(
      'module' => 'blog',
      'delta' => 'recent',
      'region' => 'sidebar_a',
      'title' => 'Blog posts',
    );

    // Prepare a menu item for the blog page.
    $data['menu_link'] = array(
      'link_path' => 'blog',
      'link_title' => 'Blog',
      'menu_name' => 'main-menu',
      'weight' => -20,
    );
  }

  /**
   * Site template callback for the "Blog" feature.
   */
  public function site_template_blog_feature_install(&$data, $features) {
    // Enable the modules.
    module_enable(array('blog', 'flexible_blogs'), FALSE);

    // Add our standard "Tags" vocabulary.
    gardens_misc_add_tags_field_instance('blog');

    // Do not promote blog posts to the front page by default.
    variable_set('node_options_blog', array('status'));

    // Show only the five newest blog posts in the blog block by default.
    variable_set('blog_block_count', 5);

    // Include blog posts in the XML Sitemap, but only if the menu that the
    // Blog link is in is also XML Sitemap enabled, or if a preparation step
    // explicitly set the 'include_blog_posts_in_xmlsitemap' variable to force
    // it to behave that way (e.g., the blog template does this).
    // @see gardens_setup_xmlsitemap()
    if (!empty($data['menu_link'])) {
      $menu_name = isset($data['menu_link']['menu_name']) ? $data['menu_link']['menu_name'] : 'navigation';
      $current_settings = variable_get("xmlsitemap_settings_menu_link_$menu_name", array());
      if (!empty($current_settings['status'])) {
        $data['include_blog_posts_in_xmlsitemap'] = TRUE;
      }
    }
    if (!empty($data['include_blog_posts_in_xmlsitemap'])) {
      variable_set('xmlsitemap_settings_node_blog', array('status' => '1', 'priority' => '0.5'));
    }

    // Create the blog post.
    $data['nids'] = array();
    foreach ($data['blog_posts'] as $blog_post) {
      $node = site_template_add_basic_node($blog_post);
      $data['nids'][] = $node->nid;
    }

    // Add the comment to the last one.
    if (!empty($data['comment']) && !empty($data['nids'])) {
      $comment_nid = end($data['nids']);
      $data['comment']->nid = $comment_nid;
      if (!isset($data['comment']->cid)) {
        $data['comment']->cid = NULL;
      }
      if (!isset($data['comment']->pid)) {
        $data['comment']->pid = NULL;
      }
      comment_save($data['comment']);
    }

    // Enable the block.
    if (!empty($data['block_data'])) {
      $block = $data['block_data'];
      if (site_template_enable_block($block)) {
        $data['block'] = $block;
      }
    }

    // Add the menu item.
    if (!empty($data['menu_link']) && ($mlid = site_template_menu_link_save($data['menu_link']))) {
      $data['mlid'] = $mlid;
    }
  }

  /**
   * Rollback function for the "Blog" feature.
   */
  public function site_template_blog_feature_rollback($data) {
    if (isset($data['mlid'])) {
      menu_link_delete($data['mlid']);
    }
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }
    foreach ($data['nids'] as $nid) {
      node_delete($nid);
    }
    module_disable(array('flexible_blogs', 'blog'), FALSE);
  }

  /**
   * Site template preparation callback for the "Contact" feature.
   */
  public function site_template_contact_feature_install_prepare(&$data, $features) {
    $data['contact_form_category'] = 'General feedback';
    $data['menu_link'] = array(
      'link_path' => 'contact',
      'link_title' => 'Contact',
      'menu_name' => 'main-menu',
      'weight' => -12,
    );
  }

  /**
   * Site template callback for the "Contact" feature.
   */
  public function site_template_contact_feature_install(&$data, $features) {
    $this->installBaseContactFeature($data, $features);

    // Add the menu item if it exists.
    if (!empty($data['menu_link']) && ($mlid = site_template_menu_link_save($data['menu_link']))) {
      $data['mlid'] = $mlid;
    }
  }

  /**
   * Rollback function for the "Contact" feature.
   */
  public function site_template_contact_feature_rollback($data) {
    site_template_restore_permissions_from_state($data['permission_states']);
    drupal_write_record('contact', $data['contact_form'], array('cid'));
    if (isset($data['mlid'])) {
      menu_link_delete($data['mlid']);
    }
    module_disable(array('contact'), FALSE);
  }

  /**
   * Site template preparation callback for the "Forum" feature.
   */
  public function site_template_forum_feature_install_prepare(&$data, $features) {
    // Define a menu link.
    $data['menu_link'] = array(
      'link_path' => 'forum',
      'link_title' => 'Forum',
      'menu_name' => 'main-menu',
      'weight' => -16,
    );

    // Define an initial forum.
    $forum = (object) array(
      'name' => 'This is a forum',
      'description' => '<p>A forum is used to group related discussion topics. Sample container names could be Announcements, or Q&A. Administrators could post important topics to the Announcements container and site visitors could post questions to the Q&A container, where other members would reply with answers.<br />To add or manage Forums, click Structure > Forums.</p><a href="http://www.drupalgardens.com/documentation/forums" target="_blank">Learn more</a>',
      'weight' => -2,
    );
    $data['forums']['discussion_forum']['forum'] = $forum;

    // Define an initial forum topic.
    $forum_topic = array(
      'type' => 'forum',
      'title' => "This is a sample forum topic",
      'body' => '<p>A forum topic is a new discussion thread. Most forum topics are either questions or announcements. Site visitors or administrators can add comments in response to the forum topic if it allows comments. Forum topics are placed into forums, and can be moved by administrators to another forum to keep discussions organized.</p><p>To add Forum Topics, click Add content  > Forum Topic.</p><p><a href="http://www.drupalgardens.com/documentation/forums" target="_blank">Learn more</a></p>',
    );
    $data['forums']['discussion_forum']['forum_topic'] = $forum_topic;

    // Define an initial comment on the forum topic.
    $data['forums']['discussion_forum']['comments'][] = site_template_prepare_basic_comment('This is a sample comment', '<p>Comments are posted by site visitors in response to forum topics.</p><p><a href="http://www.drupalgardens.com/documentation/forums" target="_blank">Learn more</a></p>');
  }

  /**
   * Site template callback for the "Forum" feature.
   */
  public function site_template_forum_feature_install(&$data, $features) {
    // Enable the module.
    module_enable(array('forum'), FALSE);

    // Add our standard "Tags" vocabulary.
    gardens_misc_add_tags_field_instance('forum');

    // Save the menu link.
    if ($mlid = site_template_menu_link_save($data['menu_link'])) {
      $data['mlid'] = $mlid;
    }

    // Add the initial forums, topics and comments.
    $forum_vocab = variable_get('forum_nav_vocabulary', '');
    $data['tids'] = array();
    $data['nids'] = array();


    // Remove the forum that comes with d.o.
    foreach (taxonomy_get_term_by_name('General discussion') as $term) {
      if ($term->vid == $forum_vocab) {
        taxonomy_term_delete($term->tid);
        break;
      }
    }

    foreach ($data['forums'] as $forum_data) {
      $forum_data['forum']->vid = $forum_vocab;
      taxonomy_term_save($forum_data['forum']);
      // Unfortunately, taxonomy_term_save() does not tell us the newly-saved
      // term IDs, so we need to grab it via a bad hack.
      $tid = db_query("SELECT MAX(tid) FROM {taxonomy_term_data}")->fetchField();
      $data['tids'][] = $tid;
      $forum_data['forum_topic']['fields'] = array(
        'taxonomy_forums' => array(
          array('tid' => $tid),
        ),
      );
      $node = site_template_add_basic_node($forum_data['forum_topic']);
      $data['nids'][] = $node->nid;
      foreach ($forum_data['comments'] as $comment) {
        $comment->nid = $node->nid;
        if (!isset($comment->cid)) {
          $comment->cid = NULL;
        }
        if (!isset($comment->pid)) {
          $comment->pid = NULL;
        }
        comment_save($comment);
      }
    }
  }

  /**
   * Rollback function for the "Forum" feature.
   */
  public function site_template_forum_feature_rollback($data) {
    foreach ($data['nids'] as $nid) {
      node_delete($nid);
    }
    foreach ($data['tids'] as $tid) {
      taxonomy_term_delete($tid);
    }
    if (!empty($data['mlid'])) {
      menu_link_delete($data['mlid']);
    }
    module_disable(array('forum'), FALSE);
  }

  /**
   * Site template preparation callback for the "About" page.
   */
  public function site_template_about_page_install_prepare(&$data, $features) {
    // Prepare a page with an embedded Google map.
    $default_map_link = 'http://maps.google.com/maps?q=map+Hilda+Ramstraat+39+2600+Berchem+Belgium'
      . '&amp;oe=utf-8&amp;ie=UTF8&amp;hq=&amp;hnear=Hilda+Ramstraat+39,+Berchem+2600+Antwerp,+Flemish+Region,+Belgium'
      . '&amp;gl=us&amp;ei=1XfHS4O5LsGBlAeQwLzFAQ&amp;ved=0CAoQ8gEwAA&amp;ll=51.188752,4.421507&amp;spn=0.002122,0.005423'
      . '&amp;t=h&amp;z=18&amp;source=embed';
    $google_map = '<iframe width="100%" height="350" frameborder="0" scrolling="no" marginheight="0" marginwidth="0" '
      . 'src="http://maps.google.com/maps?q=map+Hilda+Ramstraat+39+2600+Berchem+Belgium&amp;oe=utf-8'
      . '&amp;ie=UTF8&amp;hq=&amp;hnear=Hilda+Ramstraat+39,+Berchem+2600+Antwerp,+Flemish+Region,+Belgium'
      . '&amp;gl=us&amp;ei=1XfHS4O5LsGBlAeQwLzFAQ&amp;ved=0CAoQ8gEwAA&amp;ll=51.188752,4.421507'
      . '&amp;spn=0.002122,0.005423&amp;t=h&amp;z=18&amp;output=embed"></iframe><br /><small>'
      . '<a href="' . $default_map_link . '">View Larger Map</a></small>';
    $text = "<p>The About page is a Basic page customized to provide information about you or your organization. This basic page contains an embedded Google map and has a link to the About menu. You can edit this page to suit your needs, including changing the map location to your location.</p><p>To change the embedded map location:</p><ul><li>Open a new browser window and find a replacement location on http://maps.google.com.</li><li>When you have found a map, select \"Link\" at the top of the map, and follow the provided instructions.</li><li>On this page, click the Edit tab.</li><li>Click the HTML tab and paste the map copied from Google into the page body, replacing the old map.</li></ul><p>To add another Basic page, click Add content > Basic page.</p><p><a href='http://www.drupalgardens.com/learnmore/aboutus' target='_blank'>Learn more</a></p>$google_map";

    // Prepare the "About" page as a full HTML node.
    $full_html = db_query("SELECT format FROM {filter_format} WHERE name = 'Full HTML'")->fetchField();
    $data['page'] = array(
      'title' => 'About us',
      'body' => $text,
      'format' => $full_html,
      'menu_link' => array(
        'menu_name' => 'main-menu',
        'link_title' => 'About',
        'weight' => -24,
      ),
      'alias' => 'about-us',
    );
  }

  /**
   * Site template callback for the "About" page.
   */
  public function site_template_about_page_install(&$data, $features) {
    // Save the page.
    $data['nid'] = site_template_add_basic_node($data['page']);
  }

  /**
   * Rollback function for the "About" page.
   */
  public function site_template_about_page_rollback($data) {
    node_delete($data['nid']);
  }

  /**
   * Site template preparation callback for the "News" page.
   */
  public function site_template_news_page_install_prepare(&$data, $features) {
    // Prepare a listing page.
    $simpleview = array(
      'path' => 'news',
      'title' => 'News',
      'filter' => 'node:article',
      'style' => 'teasers',
      'sort' => 'newest',
      'rss' => 1,
      'page_items' => 10,
      'block_items' => 0,
      'block_style' => 'titles',
      'menu_link' => array(
        'menu_name' => 'main-menu',
        'weight' => -18,
      ),
    );
    $data['simpleview_to_create'] = $simpleview;

    // Prepare content for the listing page. The numbering depends on whether
    // we're installing the sample article feature also, since that creates a
    // news item too.
    $article_label = array('sample', 'second', 'third');
    if (in_array('site_template_sample_article', $features)) {
      array_shift($article_label);
    }
    $article = array(
      'type' => 'article',
      'title' => "This is a {$article_label[0]} news item",
      'body' => "<p>News items are intended for content such as announcements, press releases and other time-specific information. News items are one of many content types you can display on your site.</p><p>To change your site's front page, click Configuration > Site Information.<br />To add another News item, click Add content > News item.</p><a href='http://www.drupalgardens.com/learnmore/news' target='_blank'>Learn more</a>",
    );
    $data['articles'][] = $article;
    $article['title'] = "This is a {$article_label[1]} news item";
    $data['articles'][] = $article;
  }

  /**
   * Site template callback for the "News" page.
   */
  public function site_template_news_page_install(&$data, $features) {
    // Rename the core article content type to "News item".
    $type = node_type_load('article');
    $type->name = 'News item';
    $type->description = 'Use <em>news items</em> for time-specific content like press releases or announcements.';
    node_type_save($type);

    // Create the listing page.
    $data['simpleview'] = site_template_create_simpleview($data['simpleview_to_create']);

    // Create the articles.
    $data['nids'] = array();
    foreach ($data['articles'] as $article) {
      $node = site_template_add_basic_node($article);
      $data['nids'][] = $node->nid;
    }
  }

  /**
   * Rollback function for the "News" page.
   */
  public function site_template_news_page_rollback($data) {
    site_template_delete_simpleview($data['simpleview']);
    foreach ($data['nids'] as $nid) {
      node_delete($nid);
    }
  }

  /**
   * Site template preparation callback for the "Mailing list" feature.
   */
  public function site_template_mailing_list_feature_install_prepare(&$data, $features) {
    // Prepare a mailing list.
    $default_list_name = variable_get('site_template_mailing_list_feature_default_name', 'Join our mailing list');
    $list = new StdClass();
    $list->name = $default_list_name;
    $data['list_to_create'] = $list;

    // Prepare the mailing list block in a default location.
    $data['block_data'] = array(
      'module' => 'mailing_list',
      'region' => 'sidebar_a',
      'title' => 'Join our mailing list',
    );
  }

  /**
   * Site template callback for the "Mailing list" feature.
   */
  public function site_template_mailing_list_feature_install(&$data, $features) {
    // Enable the module.
    module_enable(array('mailing_list'), FALSE);

    // Save some variables.
    $variables = array('mailing_list_subscription_message');
    $data['variable_states'] = site_template_get_variable_states($variables);
    variable_set('mailing_list_subscription_message', 'Thank you for you interest, "%name <%mail> has been subscribed.');

    // Create the mailing list.
    mailing_list_save($data['list_to_create']);
    if (empty($data['list_to_create']->mlid)) {
      watchdog('site_template', 'Unable to create mailing list, not configuring block', 'error');
      return;
    }
    $data['mailing_list_id'] = $data['list_to_create']->mlid;

    // Enable the block.
    $block = $data['block_data'];
    $block['delta'] = $data['mailing_list_id'];
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
  }

  /**
   * Rollback function for the "Mailing list" feature.
   */
  public function site_template_mailing_list_feature_rollback($data) {
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }
    // TODO: Figure out if we should really delete the mailing list here (which
    // deletes all emails), and if we should uninstall all modules (which
    // deletes all subscriptions), etc.
    mailing_list_delete($data['mailing_list_id']);
    site_template_restore_variables_from_state($data['variable_states']);
    module_disable(array('mailing_list'), FALSE);
  }

  /**
   * Site template callback for the "Rotating banner" feature.
   */
  public function site_template_rotating_banner_feature_install(&$data, $features) {
    // Enable the module.
    module_enable(array('rotating_banner'), FALSE);

    // Add the default images to their files table... (should we even be storing FID?  perhaps just the url is better...).
    // But then, it won't show up in the media browser, which for consistancy, I suppose it should... tough call.
    $first_default_image = site_template_copy_asset('rotating_banner', 'sample-banner.png');
    $second_default_image = site_template_copy_asset('rotating_banner', 'sample-banner-2.png');
    variable_set('rotating_banner_default_slide_fid', $first_default_image->fid);

    // Now create the banner.
    $rb = RotatingBanner::create('Main banner');
    $configure_link = 'admin/structure/block/manage/rotating_banner/' . $rb->rbid . '/configure';

    // Create a slide.
    $slide = RotatingBannerSlide::create($rb->rbid);

    $slide->textboxes[0]['content'] = t('This is a Rotating Banner');
    $slide->textboxes[1]['content'] = t("Rotating Banners display one or more images with overlaid text. You can customize how they rotate, their buttons, and where they are placed.<br />To add another Rotating banner, click Structure > Blocks > Add a rotating banner.<br />Click to learn more.");
    $slide->link = 'http://www.drupalgardens.com/learnmore/rotatingbanner';
    $slide->fid = $first_default_image->fid;
    $slide->save();

    // Create a slide.
    $slide = RotatingBannerSlide::create($rb->rbid);

    $slide->textboxes[0]['content'] = t('This is another Rotating Banner');
    $slide->textboxes[1]['content'] = t("Rotating Banners display one or more images with overlaid text. You can customize how they rotate, their buttons, and where they are placed.<br />To add another Rotating banner, click Structure > Blocks > Add a rotating banner.<br />Click to learn more.");
    $slide->link = 'http://www.drupalgardens.com/learnmore/rotatingbanner';
    $slide->fid = $second_default_image->fid;
    $slide->save();

    $block_vars = array(
      'module' => 'rotating_banner',
      'delta' => $rb->rbid,
      'region' => 'banner',
      'pages' => array('<front>'),
    );
    // Place it in the correct position
    site_template_enable_block($block_vars);
  }

  /**
   * Rollback function the "Rotating banner" feature.
   */
  public function site_template_rotating_banner_feature_rollback($data) {
    // I guess we don't really want to rollback the sample-image because it might
    // be in use.
    module_disable(array('rotating_banner'), FALSE);
  }

  /**
   * Site template preparation callback for the "FAQ" feature.
   */
  public function site_template_faq_feature_install_prepare(&$data, $features) {
    // Prepare the FAQ item content type.
    $data['node_type_data'] = array(
      'type' => 'faq_item',
      'name' => 'FAQ item',
      'description' => 'Use a <em>FAQ item</em> to provide a question and answer about your site.',
      'title_label' => 'Question',
      'body_label' => 'Answer',
    );

    // Prepare some initial FAQ items.
    $post = array(
      'type' => 'faq_item',
      'title' => 'This is a sample FAQ. What does a FAQ do?',
      'body' => 'FAQs (or Frequently Asked Questions) display a list of common questions for your site visitors. Clicking any question reveals the answer. FAQs allow site visitors to quickly find answers to their questions without consuming your time. FAQs are a custom content type created by the FAQ feature.</p><p>To add FAQs, click Add content > FAQ.</p><p><a href="http://www.drupalgardens.com/learnmore/samplefaq" target="_blank">Learn more</a>',
    );
    $data['faq_items'][] = $post;
    $post['title'] = 'This is a sample FAQ. What does a FAQ do?';
    $data['faq_items'][] = $post;

    // Prepare a listing page for it, with a menu item.
    $simpleview = array(
      'path' => 'faq',
      'title' => 'Frequently asked questions',
      'filter' => 'node:faq_item',
      'style' => 'title_reveals_full',
      'sort' => 'newest',
      'rss' => 1,
      'page_items' => 10,
      'block_items' => 0,
      'block_style' => 'titles',
      'menu_link' => array(
        'link_title' => 'FAQ',
        'menu_name' => 'main-menu',
        'weight' => -14,
      ),
    );
    $data['simpleview_to_create'] = $simpleview;
  }

  /**
   * Site template callback for the "FAQ" feature.
   */
  public function site_template_faq_feature_install(&$data, $features) {
    // Save the content type.
    site_template_add_node_type($data['node_type_data']);

    // Do not show the "Share this" links on it. This is just a small variable,
    // so it's worth doing even if the "Share this" feature wasn't installed
    // (since someone might install it later and we want to give them reasonable
    // default behavior).
    variable_set('addthis_nodetype_' . $data['node_type_data']['type'], 0);

    // Save the initial FAQ items.
    $data['nids'] = array();
    foreach ($data['faq_items'] as $faq_item) {
      $node = site_template_add_basic_node($faq_item);
      $data['nids'][] = $node->nid;
    }

    // Save the listing page.
    $data['simpleview'] = site_template_create_simpleview($data['simpleview_to_create']);
  }

  /**
   * Rollback function for the "FAQ" feature.
   */
  public function site_template_faq_feature_rollback($data) {
    // TODO: Delete the content type also?
    site_template_delete_simpleview($data['simpleview']);
    foreach ($data['nids'] as $nid) {
      node_delete($nid);
    }
  }

  /**
   * Site template preparation callback for the Twitter feature.
   */
  public function site_template_twitter_feature_install_prepare(&$data, $features) {
    // Prepare the Twitter feed.
    $data['feed'] = array(
      'title' => 'Overheard on Twitter',
      'url' => 'http://twitter.com/statuses/user_timeline/132262589.rss',
      'refresh' => 3600,
      'block' => 5,
    );

    // Prepare the block.
    $data['block_data'] = array(
      'module' => 'aggregator',
      'region' => 'sidebar_b',
    );
  }

  /**
   * Site template callback for the Twitter feature.
   */
  public function site_template_twitter_feature_install(&$data, $features) {
    // Enable the aggregator module.
    module_enable(array('aggregator'), FALSE);

    // Set the aggregator to never clear entries so this block will stick around.
    variable_set('aggregator_clear', AGGREGATOR_CLEAR_NEVER);

    // Save the feed.
    aggregator_save_feed($data['feed']);
    $data['fid'] = db_query("SELECT MAX(fid) FROM {aggregator_feed}")->fetchField();

    // Refresh the feed so that it starts off with items in it immediately
    // after this template is installed (rather than, e.g., waiting for cron).
    aggregator_refresh(aggregator_feed_load($data['fid']));

    // Save the block.
    $block = $data['block_data'];
    $block['delta'] = 'feed-' . $data['fid'];
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
  }

  /**
   * Rollback function the Twitter feature.
   */
  public function site_template_twitter_feature_rollback($data) {
    // Delete the block.
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }

    // Delete the feed.
    aggregator_save_feed(array('fid' => $data['fid']));

    // Disable the module.
    module_disable(array('aggregator'), FALSE);
  }

  /**
   * Site template callback for the "Webforms" feature.
   */
  public function site_template_webforms_feature_install(&$data, $features) {
    // Enable the appropriate modules, ordered according to dependencies.
    $modules = array(
      'webform',
      'ux_elements',
      'options_element',
      'form_builder',
      'form_builder_webform',
      'webform_alt_ui',
    );
    module_enable($modules, FALSE);
  }

  /**
   * Rollback function for the "Webforms" feature.
   */
  public function site_template_webforms_feature_rollback($data) {
    // TODO.
  }

  /**
   * Site template preparation callback for the "Feedback" feature.
   */
  public function site_template_feedback_feature_install_prepare(&$data, $features) {
    $data['block_data'] = array(
      'module' => 'feedback',
      'delta' => 'feedback',
      'region' => 'content',
    );
  }

  /**
   * Site template callback for the "Feedback" feature.
   */
  public function site_template_feedback_feature_install(&$data, $features) {
    $this->installBaseContactFeature($data, $features);
    module_enable(array('feedback'), FALSE);
    $block = $data['block_data'];
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
  }

  /**
   * Rollback function for the "Feedback" feature.
   */
  public function site_template_feedback_feature_rollback($data) {
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }
    module_disable(array('feedback'), FALSE);
    // TODO: Roll back the contact module also.
  }

  /**
   * Site template preparation callback for the "Gallery" feature.
   */
  public function site_template_gallery_feature_install_prepare(&$data, $features) {
    // Prepare a gallery node.
    $data['galleries'][] = array(
      'node' => array(
        'type' => 'media_gallery',
        'title' => 'Sample gallery',
        'fields' => array(
          'media_gallery_expose_block' => array(
            array(
              'value' => 1,
            ),
          ),
          'media_gallery_description' => array(
            array(
              'value' => 'This is an individual Gallery that can contain images and videos.  It is configured by default to show 4 columns and 3 rows of images or video which are called media.  To increase or decrease the thumbnail size, edit the number of columns in the gallery.   You can organize media by dragging and dropping thumbnails.  To remove media, click the “Edit media” tab above.',
              'format' => variable_get('gardens_safe_html_format'),
            ),
          ),
        ),
      ),
      'images' => array(
        array(
          'folder' => 'media_gallery',
          'filename' => 'gallery_sample_01.png',
        ),
        array(
          'folder' => 'media_gallery',
          'filename' => 'gallery_sample_02.png',
        ),
      ),
      'videos' => array(
/*

// Commenting this out until we have an actual video we want to use.

        array(
          'url' => 'http://www.youtube.com/watch?v=fdbFVWupSsw',
          'folder' => 'media_gallery',
          'filename' => 'fdbFVWupSsw.jpg',
          'public_subdir' => 'media-youtube',
        ),
        array(
          'url' => 'http://www.youtube.com/watch?v=lZ-s3DRZJKY',
          'folder' => 'media_gallery',
          'filename' => 'lZ-s3DRZJKY.jpg',
          'public_subdir' => 'media-youtube',
        ),
*/
      ),
    );

    // Prepare the block that displays the gallery node.
    $data['block_data'] = array(
      'module' => 'media_gallery',
      'region' => 'sidebar_b',
    );
  }

  /**
   * Site template callback for the "Gallery" feature.
   */
  public function site_template_gallery_feature_install(&$data, $features) {
    // Enable the appropriate modules.
    module_enable(array('media_gallery'), FALSE);

    // Save the gallery nodes and any files associated with them.
    $data['nids'] = array();
    foreach ($data['galleries'] as &$gallery) {
      $image_fids = array();
      foreach ($gallery['images'] as $image) {
        $file = site_template_copy_asset($image['folder'], $image['filename']);
        $image_fids[] = $file->fid;
        $gallery['node']['fields']['media_gallery_media'][] = array('fid' => $file->fid);
        $this->preCreateImageDerivative('public://' . $image['filename']);
      }
      $this->addTitlesAndDescriptions($image_fids, 'image');
      // Work around a static caching bug to make sure the videos are actually
      // recognized.
      drupal_static_reset('file_get_stream_wrappers');
      $video_fids = array();
      foreach ($gallery['videos'] as $video) {
        $provider = media_internet_get_provider($video['url']);
        $file = $provider->save();
        $video_fids[] = $file->fid;
        $gallery['node']['fields']['media_gallery_media'][] = array('fid' => $file->fid);
        // Copy the screenshot to the files directory before creating
        // derivatives.
        $public_subdir_uri = 'public://' . $video['public_subdir'];
        file_prepare_directory($public_subdir_uri, FILE_CREATE_DIRECTORY);
        $screenshot_uri = $public_subdir_uri . '/' . $video['filename'];
        copy(drupal_get_path('module', 'site_template')  . "/assets/{$video['folder']}/{$video['filename']}", $screenshot_uri);
        $this->preCreateImageDerivative($screenshot_uri);
      }
      $this->addTitlesAndDescriptions($video_fids, 'video');
      $node = site_template_add_basic_node($gallery['node']);
      $data['nids'][] = $node->nid;
    }

    // Enable the block; display the first gallery node we saved above.
    $block = $data['block_data'];
    $block['delta'] = reset($data['nids']);
    if (site_template_enable_block($block)) {
      $data['block'] = $block;
    }
  }

  /**
   * Rollback function for the "Gallery" feature.
   */
  public function site_template_gallery_feature_rollback($data) {
    // Disable the block.
    if (isset($data['block'])) {
      site_template_disable_block($data['block']);
    }

    // Delete any nodes that were created.
    foreach ($data['nids'] as $nid) {
      node_delete($nid);
    }

    // Disable the gallery module.
    module_disable(array('media_gallery'), FALSE);
  }

  /**
   * Pre-create image derivatives for a sample image (or video thumbnail).
   *
   * We've seen bugs where after initial site creation, the images don't all
   * show up on the first page load, and it looks like it might be a race
   * condition involving an attempt to create the style directory as the images
   * are being generated. This race condition shouldn't happen, but
   * pre-creating the image derivatives should speed up the initial page load
   * after site creation anyway, so it's worth doing. For now, we only do this
   * for the styles that the media gallery module itself provides (which is a
   * thumbnail style), since that's what we need for the initial page load.
   *
   * @param $image_uri
   *   Local URI where the original copy of the image is stored.
   */
  protected function preCreateImageDerivative($image_uri) {
    $media_gallery_styles = module_invoke('media_gallery', 'image_default_styles');
    if (!empty($media_gallery_styles)) {
      foreach (array_keys($media_gallery_styles) as $style_name) {
        $style_path = image_style_path($style_name, $image_uri);
        if (!file_exists($style_path)) {
          $style = image_style_load($style_name);
          image_style_create_derivative($style, $image_uri, $style_path);
        }
      }
    }
  }

  /**
   * Adds sequentially numbered titles and descriptions to media items.
   *
   * @param $fids
   *   An array of file IDs corresponding to the media entities to act on.
   * @param $type
   *   The type of file represented by the items in the list.
   */
  protected function addTitlesAndDescriptions($fids, $type) {
    // Only put a number in the title if there is more than one item.
    $count = count($fids) > 1 ? 0 : NULL;
    foreach ($fids as $fid) {
      $file = file_load($fid);
      $title = "Sample $type";
      if (isset($count)) {
        $count++;
        $title .= ' ' . $count;
      }
      $file->media_title[LANGUAGE_NONE][0]['value'] = $title;
      $file->media_description[LANGUAGE_NONE][0]['value'] = "This sample $type is demonstrating the Gallery feature.";
      file_save($file);
    }
  }

  /**
   * Helper function to install the basic "Contact" feature and ensure it does
   * not happen more than once.
   */
  protected function installBaseContactFeature(&$data, $features) {
    $installed = &drupal_static(__FUNCTION__, FALSE);
    if (!$installed) {
      // Enable the module.
      module_enable(array('contact'), FALSE);

      // Grant permission to view the contact form.
      $contact_permissions = array('access site-wide contact form');
      $data['permission_states'] = site_template_get_permission_states($contact_permissions);
      user_role_grant_permissions(DRUPAL_ANONYMOUS_RID, $contact_permissions);
      user_role_grant_permissions(DRUPAL_AUTHENTICATED_RID, $contact_permissions);

      // Customize the contact form appropriately.
      $contact_form = contact_load(1); // This was set up in contact.install
      $data['contact_form'] = $contact_form;
      $contact_form['category'] = $data['contact_form_category'];
      $contact_form['recipients'] = variable_get('site_mail');
      drupal_write_record('contact', $contact_form, array('cid'));

      // Add a custom block with help text.
      $custom_block = array(
        'info' => 'Contact page help',
        'body' => "This is a block placed on your site's contact form. You can edit this text and replace it with whatever you want people contacting you to see. To add contact categories and recipient e-mail addresses to your contact form, click Structure > Contact form.",
        'format' => variable_get('gardens_safe_html_format'),
      );
      $block = array(
        'module' => 'block',
        'region' => 'help',
        'pages' => array('contact'),
      );
      // Save the custom block and get its delta.
      $delta = db_insert('block_custom')
        ->fields(array(
          'body' => $custom_block['body'],
          'info' => $custom_block['info'],
          'format' => $custom_block['format'],
        ))
        ->execute();
      // Enable the block.
      $block['delta'] = $delta;
      site_template_enable_block($block);

      // Mark this feature as installed.
      $installed = TRUE;
    }
  }

  /**
   * Returns an array of basic permissions to give to "content creator" roles.
   *
   * This does not return permissions for any specific type of content; that
   * is up to the calling function to merge in.
   */
  protected function baseContentCreatorPermissions() {
    $permissions = array(
      'edit own comments',
      'edit own follow links',
      'administer media',
      'view media',
      'edit media',
      'access content overview',
      'view own unpublished content',
      'access overlay',
      'administer url aliases',
      'create url aliases',
      'vote on polls',
      'cancel own vote',
      'inspect all votes',
      'use advanced search',
      'customize shortcut links',
      'switch shortcut sets',
      'access administration pages',
      'view the administration theme',
      'access toolbar',
      'access user profiles',
    );
    if (module_exists('taxonomy')) {
      foreach (taxonomy_get_vocabularies() as $vocabulary) {
        $permissions[] = 'edit terms in ' . $vocabulary->vid;
      }
    }
    $safe_html_format_id = variable_get('gardens_safe_html_format');
    if (!empty($safe_html_format_id)) {
      $permissions[] = filter_permission_name(filter_format_load($safe_html_format_id));
    }
    return $permissions;
  }
  /**
   * Returns an array of basic permissions to give to "designer" roles.
   */
  protected function baseSiteDesignerPermissions() {
    $permissions = array(
      'administer themes',
      'access themebuilder styles tab',
      'access themebuilder brand tab',
      'access themebuilder layout tab',
      'access themebuilder advanced tab',
      'access themebuilder theme tab',
      'publish theme',
      'administer views',
      'administer media galleries',
    );
    return $permissions;
  }
}
