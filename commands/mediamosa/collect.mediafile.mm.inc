<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Implementation of drush_COMMAND().
 * Other way: drush_hook_COMMAND().
 *
 * Drush command callback. This is where the action takes place.
 *
 * If for some reason you do not want your hook function to be named
 * after your command, you may define a 'callback' item in your command
 * object that specifies the exact name of the function that should be
 * called.  However, the specified callback function must still begin
 * with "drush_commandfile_" (e.g. 'callback' => "drush_example_foo_execute").
 * All hook functions are still called (e.g. drush_example_pre_foo_execute,
 * and so on.)
 *
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 */
function drush_mediamosa_mm_mediafile_collect($subcommand = NULL) {

  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Run the command.
  _drush_mediamosa_mm_mediafile_collect();
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_mediafile_collect_validate() {
  // We don't have anything to monitor.
}

/**
 * Collect all the mediafiles.
 */
function _drush_mediamosa_mm_mediafile_collect() {
  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
  ));

  // Condition.
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Range.
  $result = $query->execute();
  foreach ($result as $record) {
    drush_print($record[mediamosa_asset_mediafile_db::ID]);
  }
}
