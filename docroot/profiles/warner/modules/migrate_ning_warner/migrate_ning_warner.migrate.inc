<?php
/**
 * @file
 * Summary
 */

/**
 * Implementation of hook_migrate_api.
 */
function migrate_ning_warner_migrate_api() {
  $common_arguments = array(
    'source_directory' => variable_get('migrate_ning_directory', ''),
    'use_remote_files' => TRUE,
  );
  $node_arguments = $common_arguments + array(
    'default_format' => '6',
    'member_migration' => 'Member',
  );
  $comment_arguments = $node_arguments;

  $migrations = array(
    'Picture' => $common_arguments + array(
      'class_name' => 'NingPictureMigration',
      'default_picture' => variable_get('migrate_ning_picture', ''),
    ),
    'Member' => $common_arguments + array(
      'class_name' => 'NingWarnerMemberMigration',
      'picture_migration' => 'Picture',
    ),
    'MemberComment' => $comment_arguments + array(
      'class_name' => 'NingMemberCommentMigration',
      'entity_migration' => 'Member',
      'bundle' => 'comment_node_comment_on_anything',
    ),
    'Page' => $node_arguments + array(
      'class_name' => 'NingWarnerPageMigration',
      'bundle' => 'blog',
    ),
    'PageComment' => $comment_arguments + array(
      'class_name' => 'NingPageCommentMigration',
      'entity_migration' => 'Page',
      'bundle' => 'comment_node_blog',
    ),
    'Blog' => $node_arguments + array(
      'class_name' => 'NingWarnerBlogMigration',
      'bundle' => 'blog',
    ),
    'BlogFan' => $node_arguments + array(
      'class_name' => 'NingWarnerBlogFanMigration',
      'bundle' => 'forum',
    ),
    'BlogComment' => $comment_arguments + array(
      'class_name' => 'NingWarnerBlogCommentMigration',
      'entity_migration' => 'Blog',
      'bundle' => 'comment_node_blog',
    ),
    'Event' => $node_arguments + array(
      'class_name' => 'NingWarnerEventMigration',
      'bundle' => 'event',
    ),
    'EventComment' => $comment_arguments + array(
      'class_name' => 'NingEventCommentMigration',
      'entity_migration' => 'Event',
      'bundle' => 'comment_node_event',
    ),
    'Photo' => $node_arguments + array(
      'class_name' => 'NingWarnerPhotoMigration',
      'bundle' => 'photo',
    ),
    'PhotoComment' => $comment_arguments + array(
      'class_name' => 'NingPhotoCommentMigration',
      'entity_migration' => 'Photo',
      'bundle' => 'comment_node_photo',
    ),
    'Video' => $node_arguments + array(
      'class_name' => 'NingWarnerVideoMigration',
      'bundle' => 'video',
    ),
    'VideoComment' => $comment_arguments + array(
      'class_name' => 'NingVideoCommentMigration',
      'entity_migration' => 'Video',
      'bundle' => 'comment_node_video',
    ),
  );

  // Skip groups/discussions for TheNewBoyz
  $site = basename(variable_get('migrate_ning_directory', ''));
  if ($site == 'GucciMane') {
    $migrations +=  array(
      'Group' => $common_arguments + array(
        'class_name' => 'NingWarnerGroupMigration',
      ),
      'Discussion' => $node_arguments + array(
        'class_name' => 'NingWarnerDiscussionMigration',
        'bundle' => 'forum',
        'group_migration' => 'Group',
      ),
      'DiscussionComment' => $comment_arguments + array(
        'class_name' => 'NingDiscussionCommentMigration',
        'entity_migration' => 'Discussion',
        'bundle' => 'comment_node_forum',
      ),
    );
  }

  $api = array(
    'api' => 2,
    'migrations' => $migrations,
  );
  return $api;
}
