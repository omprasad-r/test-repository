<?php
// $Id$

/**
 * @file
 * Definition of CampaignSiteTemplate.
 */

class CampaignSiteTemplate extends DrupalSiteTemplate {
  /**
   * Site template callback for basic campaign site features.
   */
  public function campaign_template_base_feature_install(&$data, $features) {
    // Save the variables we will be modifying below.
    $data['variable_states'] = site_template_get_variable_states(array('site_slogan', 'theme_default'));

    // Enable the site slogan.
    variable_set('site_slogan', 'This is your site’s slogan. Edit, move or remove it as needed.');

    // Change the default theme.
    gardens_misc_replace_default_theme('campaign');
  }

  /**
   * Rollback function for basic campaign site features.
   */
  public function campaign_template_base_feature_rollback($data) {
    site_template_restore_variables_from_state($data['variable_states']);
  }

  /**
   * Override the preparation step for the "Comment" feature.
   */
  public function site_template_comment_feature_install_prepare(&$data, $features) {
    parent::site_template_comment_feature_install_prepare($data, $features);
    // Restrict the recent comments block to forum and blog pages and the front
    // page only.
    $data['block_data']['pages'] = array(
      '<front>',
      'blog',
      'blog/*',
      'forum',
      'forum/*',
    );
  }
}

