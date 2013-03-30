-- ----------------------------
--  Table structure for `blog_post`
-- ----------------------------
DROP TABLE IF EXISTS `blog_post`;
CREATE TABLE `blog_post` (
  `post_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `author_id` int(10) unsigned NOT NULL,
  `node_assoc` int(10) unsigned NOT NULL,
  `post_date` int(10) unsigned NOT NULL,
  `mod_date` int(10) unsigned NOT NULL,
  `title` varchar(255) NOT NULL,
  `title_url` varchar(255) NOT NULL,
  `body` text NOT NULL,
  `category` int(11) NOT NULL,
  `num_direct_views` int(11) NOT NULL,
  `wordcount` int(11) NOT NULL,
  `pages` int(11) NOT NULL,
  `status` varchar(255) NOT NULL,
  `is_module` int(11) NOT NULL,
  `is_page` int(11) NOT NULL,
  PRIMARY KEY (`post_id`)
);

-- ----------------------------
--  Table structure for `comment`
-- ----------------------------
DROP TABLE IF EXISTS `comment`;
CREATE TABLE `comment` (
  `comment_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `author_id` int(10) unsigned NOT NULL,
  `xref_id` int(10) unsigned NOT NULL,
  `type` varchar(255) NOT NULL,
  `post_id` int(10) unsigned NOT NULL,
  `photo_id` int(11) NOT NULL,
  `video_id` int(11) NOT NULL,
  `comment` text NOT NULL,
  `title` varchar(255) NOT NULL,
  `date_added` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `view_status` varchar(255) NOT NULL,
  PRIMARY KEY (`comment_id`)
);
-- ----------------------------
--  Table structure for `core_video`
-- ----------------------------
DROP TABLE IF EXISTS `core_video`;
CREATE TABLE `core_video` (
  `video_id` int(10) unsigned NOT NULL,
  `gallery_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `uploaded_by` int(10) unsigned NOT NULL,
  `title` varchar(255) NOT NULL,
  `filename` varchar(255) NOT NULL,
  `original_filename` varchar(255) NOT NULL,
  `description` text NOT NULL,
  `status` varchar(255) NOT NULL,
  `aspect_ratio` varchar(255) NOT NULL,
  `width` int(10) unsigned NOT NULL,
  `height` int(10) unsigned NOT NULL,
  `bitrate` int(10) unsigned NOT NULL,
  `duration` int(10) unsigned NOT NULL,
  `date_uploaded` varchar(255) NOT NULL,
  `last_updated` varchar(255) NOT NULL,
  `num_views` int(10) unsigned NOT NULL,
  `preview_name` varchar(255) NOT NULL,
  `node_assoc` varchar(255) NOT NULL,
  `moderated_by` varchar(255) NOT NULL,
  `video_system` varchar(255) NOT NULL,
  `external_video_id` varchar(255) NOT NULL,
  `external_thumbnail_url` varchar(255) NOT NULL,
  `external_video_info` varchar(255) NOT NULL,
  `available_extensions` varchar(255) NOT NULL,
  PRIMARY KEY (`video_id`)
);

-- ----------------------------
--  Table structure for `discussion`
-- ----------------------------
DROP TABLE IF EXISTS `discussion`;
CREATE TABLE `discussion` (
  `node_id` int(10) unsigned NOT NULL,
  `discussion_id` int(10) unsigned NOT NULL,
  `num_replies` int(10) unsigned NOT NULL,
  `first_post_id` int(10) unsigned NOT NULL,
  `last_post_id` int(10) unsigned NOT NULL,
  `created_by` int(10) unsigned NOT NULL,
  `date_created` varchar(255) NOT NULL,
  `public` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `num_views` int(10) unsigned NOT NULL,
  PRIMARY KEY (`discussion_id`)
);

-- ----------------------------
--  Table structure for `discussion_comment`
-- ----------------------------
DROP TABLE IF EXISTS `discussion_comment`;
CREATE TABLE `discussion_comment` (
  `node_id` int(10) unsigned NOT NULL,
  `post_id` int(11) NOT NULL,
  `discussion_id` int(11) NOT NULL,
  `title` varchar(255) NOT NULL,
  `content` text NOT NULL,
  `user_ip` varchar(255) NOT NULL,
  `created_by` int(10) unsigned NOT NULL,
  `date_created` varchar(255) NOT NULL,
  `public` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `rating` int(10) unsigned NOT NULL,
  PRIMARY KEY (`post_id`)
);

-- ----------------------------
--  Table structure for `forum_post`
-- ----------------------------
DROP TABLE IF EXISTS `forum_post`;
CREATE TABLE `forum_post` (
  `post_id` int(10) unsigned NOT NULL,
  `thread_id` int(10) unsigned NOT NULL,
  `forum_id` int(10) unsigned NOT NULL,
  `node_id` int(10) unsigned NOT NULL,
  `post_title` varchar(255) NOT NULL,
  `post_content` text NOT NULL,
  `date_created` varchar(255) NOT NULL,
  `created_by` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `num_views` int(10) unsigned NOT NULL,
  `scaled_rating` int(10) unsigned NOT NULL,
  PRIMARY KEY (`post_id`)
);

-- ----------------------------
--  Table structure for `forum_topic_lookup`
-- ----------------------------
DROP TABLE IF EXISTS `forum_topic_lookup`;
CREATE TABLE `forum_topic_lookup` (
  `date_added` datetime NOT NULL,
  `ftl_id` int(10) unsigned NOT NULL,
  `system` varchar(32) NOT NULL,
  `topic_id` int(10) unsigned NOT NULL,
  `type` varchar(32) NOT NULL,
  `xref_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ftl_id`)
);

-- ----------------------------
--  Table structure for `forum_thread`
-- ----------------------------
DROP TABLE IF EXISTS `forum_thread`;
CREATE TABLE `forum_thread` (
  `thread_id` int(10) unsigned NOT NULL,
  `forum_id` int(10) unsigned NOT NULL,
  `forum_category_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `node_id` int(10) unsigned NOT NULL,
  `thread_title` varchar(255) NOT NULL,
  `first_post_id` int(11) NOT NULL,
  `last_post_id` int(10) unsigned NOT NULL,
  `sticky` int(10) unsigned NOT NULL,
  `anonymous` int(10) unsigned NOT NULL,
  `num_posts` int(10) unsigned NOT NULL,
  `expires` varchar(255) NOT NULL,
  `last_hot_date` varchar(255) NOT NULL,
  `created_by` int(10) unsigned NOT NULL,
  `date_created` varchar(255) NOT NULL,
  `date_created_rev` int(11) NOT NULL,
  `num_views` int(10) unsigned NOT NULL,
  `scaled_rating` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `is_public` int(10) unsigned NOT NULL,
  `owner` varchar(255) NOT NULL,
  `moderator` varchar(255) NOT NULL,
  `friend` varchar(255) NOT NULL,
  `group_member` varchar(255) NOT NULL,
  `member` varchar(255) NOT NULL,
  `non_member` varchar(255) NOT NULL,
  PRIMARY KEY (`thread_id`)
);

-- ----------------------------
--  Table structure for `forums`
-- ----------------------------
DROP TABLE IF EXISTS `forums`;
CREATE TABLE `forums` (
  `forum_id` int(10) unsigned NOT NULL,
  `forum_category_id` int(10) unsigned NOT NULL,
  `parent_id` int(10) unsigned NOT NULL,
  `children` varchar(255) NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `node_id` int(10) unsigned NOT NULL,
  `forum_title` varchar(255) NOT NULL,
  `forum_description` varchar(255) NOT NULL,
  `last_post_id` int(10) unsigned NOT NULL,
  `num_threads` int(10) unsigned NOT NULL,
  `num_posts` int(10) unsigned NOT NULL,
  `expires` varchar(255) NOT NULL,
  `created_by` varchar(255) NOT NULL,
  `date_created` int(10) unsigned NOT NULL,
  `num_views` int(10) unsigned NOT NULL,
  `scaled_rating` int(10) unsigned NOT NULL,
  `status` varchar(255) NOT NULL,
  `owner` varchar(255) NOT NULL,
  `moderator` varchar(255) NOT NULL,
  `friend` varchar(255) NOT NULL,
  `group_member` varchar(255) NOT NULL,
  `member` varchar(255) NOT NULL,
  `non_member` varchar(255) NOT NULL,
  PRIMARY KEY (`forum_id`)
);

-- ----------------------------
--  Table structure for `photo`
-- ----------------------------
DROP TABLE IF EXISTS `photo`;
CREATE TABLE `photo` (
  `photo_id` varchar(255) NOT NULL,
  `owner_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `filename` varchar(255) NOT NULL,
  `caption` varchar(255) NOT NULL,
  `viewable` varchar(255) NOT NULL,
  `gallery` int(10) unsigned NOT NULL,
  `content_status` varchar(255) NOT NULL,
  PRIMARY KEY (`photo_id`)
);

-- ----------------------------
--  Table structure for `photo_gal`
-- ----------------------------
DROP TABLE IF EXISTS `photo_gal`;
CREATE TABLE `photo_gal` (
  `blog_id` int(10) unsigned NOT NULL,
  `gallery_id` int(10) unsigned NOT NULL,
  `parent_gallery` int(10) unsigned NOT NULL,
  `gallery_name` varchar(255) NOT NULL,
  `description` varchar(255) NOT NULL,
  `gallery_viewable` varchar(255) NOT NULL,
  `photos_viewable` varchar(255) NOT NULL,
  `front_custom` varchar(255) NOT NULL,
  `front_photo` varchar(255) NOT NULL,
  PRIMARY KEY (`gallery_id`)
);

-- ----------------------------
--  Table structure for `prof_ext_entry`
-- ----------------------------
DROP TABLE IF EXISTS `prof_ext_entry`;
CREATE TABLE `prof_ext_entry` (
  `user_id` int(10) unsigned NOT NULL,
  `prof_ext_id` int(10) unsigned NOT NULL,
  `field_id` int(10) unsigned NOT NULL,
  `field_name` varchar(255) NOT NULL,
  `int_value` int(11) NOT NULL,
  `char_value` varchar(255) NOT NULL,
  PRIMARY KEY (`user_id`,`field_id`)
);

-- ----------------------------
--  Table structure for `prof_ext_fulltext`
-- ----------------------------
DROP TABLE IF EXISTS `prof_ext_fulltext`;
CREATE TABLE `prof_ext_fulltext` (
  `user_id` int(10) unsigned NOT NULL,
  `prof_ext_id` int(10) unsigned NOT NULL,
  `field_id` int(10) unsigned NOT NULL,
  `field_name` varchar(255) NOT NULL,
  `text_value` text NOT NULL,
  PRIMARY KEY (`user_id`,`field_id`)
);

-- ----------------------------
--  Table structure for `profile`
-- ----------------------------
DROP TABLE IF EXISTS `profile`;
CREATE TABLE `profile` (
  `user_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `username` varchar(255) NOT NULL,
  `domain` varchar(255) NOT NULL,
  `first_name` varchar(255) NOT NULL,
  `last_name` varchar(255) NOT NULL,
  `display_name` varchar(255) NOT NULL,
  `profile_width` int(10) unsigned NOT NULL,
  `profile_height` int(10) unsigned NOT NULL,
  `dob_mo` int(10) unsigned NOT NULL,
  `dob_day` int(10) unsigned NOT NULL,
  `dob_year` int(10) unsigned NOT NULL,
  `dob` int(11) NOT NULL,
  `dob_display` varchar(255) NOT NULL,
  `gender` int(10) unsigned NOT NULL,
  `gender1` varchar(255) NOT NULL,
  `orientation` int(10) unsigned NOT NULL,
  `loc_city` varchar(255) NOT NULL,
  `loc_state` varchar(255) NOT NULL,
  `loc_country` varchar(255) NOT NULL,
  `loc_zip` varchar(255) NOT NULL,
  `loc_zip2` varchar(255) NOT NULL,
  `loc_latitude` int(10) unsigned NOT NULL,
  `loc_longitude` int(10) unsigned NOT NULL,
  `loc_custom` varchar(255) NOT NULL,
  `home_city` varchar(255) NOT NULL,
  `home_state` varchar(255) NOT NULL,
  `home_country` int(10) unsigned NOT NULL,
  `home_zip` int(10) unsigned NOT NULL,
  `school_name` varchar(255) NOT NULL,
  `school_state` varchar(255) NOT NULL,
  `school_city` varchar(255) NOT NULL,
  `student_email` varchar(255) NOT NULL,
  `student_classification` varchar(255) NOT NULL,
  `school_major` varchar(255) NOT NULL,
  `for_dating` int(10) unsigned NOT NULL,
  `for_friendships` int(10) unsigned NOT NULL,
  `for_relationships` int(10) unsigned NOT NULL,
  `for_hookups` int(10) unsigned NOT NULL,
  `for_networking` int(10) unsigned NOT NULL,
  `religion` int(10) unsigned NOT NULL,
  `here_for` int(10) unsigned NOT NULL,
  `relationship_status` int(10) unsigned NOT NULL,
  `marital_status1` varchar(255) NOT NULL,
  `children_status` int(10) unsigned NOT NULL,
  `children_number` int(10) unsigned NOT NULL,
  `ethnicity` int(10) unsigned NOT NULL,
  `body_type` int(10) unsigned NOT NULL,
  `height` int(10) unsigned NOT NULL,
  `smoke` int(10) unsigned NOT NULL,
  `drink` int(10) unsigned NOT NULL,
  `skype_id` varchar(255) NOT NULL,
  `aim_id` varchar(255) NOT NULL,
  `yahoo_id` varchar(255) NOT NULL,
  `msn_id` varchar(255) NOT NULL,
  `gmail_id` varchar(255) NOT NULL,
  `hidden` int(10) unsigned NOT NULL,
  `deleted` int(10) unsigned NOT NULL,
  `date_created` int(10) unsigned NOT NULL,
  `date_updated` int(10) unsigned NOT NULL,
  `portrait` varchar(255) NOT NULL,
  PRIMARY KEY (`user_id`)
);

-- ----------------------------
--  Table structure for `tags`
-- ----------------------------
DROP TABLE IF EXISTS `tags`;
CREATE TABLE `tags` (
  `tag_content_id` int(10) unsigned NOT NULL,
  `content_id` int(10) unsigned NOT NULL,
  `tag_id` int(10) unsigned NOT NULL,
  `tag_date` varchar(255) NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  `ip_address` varchar(255) NOT NULL,
  `tag_id2` int(10) unsigned NOT NULL,
  `tag` varchar(255) NOT NULL,
  `xref_id` int(10) unsigned NOT NULL,
  `type` varchar(255) NOT NULL,
  PRIMARY KEY (`tag_content_id`)
);

-- ----------------------------
--  Table structure for `user`
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `user_id` int(10) unsigned NOT NULL,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `encoded_or_hashed_password` varchar(255) DEFAULT NULL,
  `email` varchar(255) NOT NULL,
  `domain` varchar(255) DEFAULT NULL,
  `subdir` varchar(255) DEFAULT NULL,
  `date_created` int(10) unsigned DEFAULT NULL,
  `last_login` int(10) unsigned DEFAULT NULL,
  `last_activity` int(10) unsigned DEFAULT NULL,
  `account_status` varchar(255) DEFAULT NULL,
  `network_id` int(10) unsigned DEFAULT NULL,
  `node_id` int(10) unsigned DEFAULT NULL,
  `first_name` varchar(255) DEFAULT NULL,
  `last_name` varchar(255) DEFAULT NULL,
  `display_name` varchar(255) DEFAULT NULL,
  `dob_month` int(10) unsigned DEFAULT NULL,
  `dob_day` int(10) unsigned DEFAULT NULL,
  `dob_year` int(10) unsigned DEFAULT NULL,
  `dob` int(11) DEFAULT NULL,
  `dob_display` varchar(255) DEFAULT NULL,
  `gender` int(11) DEFAULT NULL,
  `loc_custom` varchar(255) DEFAULT NULL,
  `personal_quote` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
);

-- ----------------------------
--  Table structure for `video`
-- ----------------------------
DROP TABLE IF EXISTS `video`;
CREATE TABLE `video` (
  `video_id` int(10) unsigned NOT NULL,
  `gallery_id` int(10) unsigned NOT NULL,
  `blog_id` int(10) unsigned NOT NULL,
  `title` varchar(255) NOT NULL,
  `filename` varchar(255) NOT NULL,
  `description` text NOT NULL,
  PRIMARY KEY (`video_id`)
);
