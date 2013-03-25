<?php
// $Id$

/**
 * @file
 * Definition of BlogSiteTemplate.
 */

class BlogSiteTemplate extends DrupalSiteTemplate {
  /**
   * Site template callback for basic blog site features.
   */
  public function blog_template_base_feature_install(&$data, $features) {
    // Save the variables we will be modifying below.
    $data['variable_states'] = site_template_get_variable_states(array('theme_default'));

    // Change the default theme.
    gardens_misc_replace_default_theme('builderbase');

    // Move the search block to the preheader.
    $block = array(
      'module' => 'search',
      'delta' => 'form',
      'region' => 'preheader_second',
      'weight' => 20,
    );
    site_template_enable_block($block);

    // Move the user menu block to sidebar A.
    $block = array(
      'module' => 'system',
      'delta' => 'user-menu',
      'region' => 'sidebar_a',
			'weight' => '20',
			'title' => '<none>',
    );
    site_template_enable_block($block);

    // Display 5 posts on the front page (and other pages as well, e.g. the
    // blog page, wherever it lives).
    variable_set('default_nodes_main', 5);
  }

  /**
   * Rollback function for basic blog site features.
   */
  public function blog_template_base_feature_rollback($data) {
    site_template_restore_variables_from_state($data['variable_states']);
  }

  /**
   * Override the preparation step for the "Blog" feature.
   */
  public function site_template_blog_feature_install_prepare(&$data, $features) {
    parent::site_template_blog_feature_install_prepare($data, $features);
    // Display the recent blog posts block in sidebar B and modify its title.
    $data['block_data']['region'] = 'sidebar_b';
    $data['block_data']['title'] = 'Recent posts';
    // Do not put a blog link in the user menu.
    unset($data['menu_link']);
    // Make sure that all the blog posts which will be created are promoted to
    // the front page.
    foreach ($data['blog_posts'] as &$post) {
      $post['promote'] = NODE_PROMOTED;
    }
    // Force blog posts to initially be included in the XML sitemap.
    $data['include_blog_posts_in_xmlsitemap'] = TRUE;
  }

  /**
   * Override the installation step for the "Blog" feature.
   */
  public function site_template_blog_feature_install(&$data, $features) {
    parent::site_template_blog_feature_install($data, $features);

    // Use the default options for blogs (which allows them to be promoted to
    // the front page) - this is only relevant if they move the blog page off
    // the front page, though.
    variable_del('node_options_blog');

    // Set the front page to point to the blog.
    variable_set('site_frontpage', 'blog');

    // Do not display links to the blog on each blog post (since the whole site
    // is designed to be a blog).
    variable_set('flexible_blogs_node_link_type', FLEXIBLE_BLOGS_NODE_LINK_NONE);
  }

  /**
   * Override the preparation step for the "Comment" feature.
   */
  public function site_template_comment_feature_install_prepare(&$data, $features) {
    parent::site_template_comment_feature_install_prepare($data, $features);
    // Display the recent comments block in sidebar B and modify its title.
    $data['block_data']['region'] = 'sidebar_b';
    unset($data['block_data']['title']);
  }

  /**
   * Override the preparation step for the "Follow us" feature.
   */
  public function site_template_follow_us_feature_install_prepare(&$data, $features) {
    parent::site_template_follow_us_feature_install_prepare($data, $features);
    // Display the Follow Us block in sidebar A and modify its title.
    $data['block_data']['region'] = 'sidebar_a';
    $data['block_data']['title'] = 'Follow me';
		$data['block_data']['weight'] = '10';
    // Add a LinkedIn link to the default Follow links.
    $link = new stdClass();
    $link->name = 'linkedin';
    $link->url = 'http://www.linkedin.com/companies/acquia';
    $link->weight = 0;
    $data['links_to_save']['linkedin'] = $link;
  }

  /**
   * Override the installation step for the "Follow us" feature.
   */
  public function site_template_follow_us_feature_install(&$data, $features) {
    parent::site_template_follow_us_feature_install($data, $features);
    // Delete the initial RSS link which the module provided.
    $lid = db_query("SELECT lid FROM {follow_links} WHERE name = 'this-site'")->fetchField();
    if ($lid) {
      follow_link_delete($lid);
    }
  }

  /**
   * Override the preparation step for the Twitter feature.
   */
  public function site_template_twitter_feature_install_prepare(&$data, $features) {
    parent::site_template_twitter_feature_install_prepare($data, $features);
    // Modify the name of the Twitter feed.
    $data['feed']['title'] = 'Heard on Twitter';
    // Display the Twitter block in sidebar A.
    $data['block_data']['region'] = 'sidebar_a';
		$data['block_data']['weight'] = '15';
  }

  /**
   * Site template callback for removing the default article type.
   *
   * Note that we want this to run last, in case the article type was used for
   * anything else (e.g., copying its default fields instances to other content
   * types) earlier in the installation.
   */
  public function blog_template_remove_article_type_install(&$data, $features) {
    // Delete the article content type, unless we are using it.
    if (!array_intersect($features, array('site_template_sample_article', 'site_template_news_page'))) {
      node_type_delete('article');
    }
  }
}

