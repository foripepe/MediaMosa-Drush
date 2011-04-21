<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Required files.
 */
require_once 'drush_response.inc';

/**
 * Constants.
 */
define('DRUSH_STATISTICS_DISKUSAGE_MAXQUERY', 1000);

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
function drush_mediamosa_mm_stats_du($subcommand = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Get the options.
  $app_ids = explode(',', drush_get_option('app_ids'));

  $drush_response = new drush_response();

  // Collect the information.
  _drush_mediamosa_mm_stats_du_collect($drush_response, $app_ids);

  // Return the information.
  $output = $drush_response->generate_xml();
  drush_print($output);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_stats_du_validate() {
}

/**
 * Collcet the disk usage information.
 *
 * @param $drush_output
 *   The output class.
 *
 * @param $app_ids
 *   Application ids.
 */
function _drush_mediamosa_mm_stats_du_collect($drush_response, $app_ids = NULL) {
  // Prop ids.
  $prop_id_filesize = mediamosa_asset_mediafile_metadata_property::get_property_id_int('filesize');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $filesize = $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'filesize', 'mediafile.' . mediamosa_asset_mediafile_db::ID . ' = filesize.' . mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID);
  $quota = $query->join(mediamosa_app_db::TABLE_NAME, 'quota', 'mediafile.' . mediamosa_asset_mediafile_db::APP_ID . ' = ' . 'quota.' . mediamosa_app_db::APP_ID);

  // Expressions.
  $query->addExpression('SUM(filesize.' . mediamosa_asset_mediafile_metadata_db::VAL_INT . ')', 'sum_filesize');

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::APP_ID,
  ));
  $query->fields('quota', array(
    mediamosa_app_db::QUOTA,
  ));

  // Condition.
  $query
    ->condition('filesize.' . mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id_filesize)
    ->isNotNull('filesize.' . mediamosa_asset_mediafile_metadata_db::VAL_INT);

  // Condition for application ids.
  if ($app_ids && is_array($app_ids) && !empty($app_ids)) {
    $or = db_or();
    $is_app_id = FALSE;
    foreach ($app_ids as $app_id) {
      if (is_numeric($app_id)) {
        $or->condition('mediafile.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
        $is_app_id = TRUE;
      }
    }
    if ($is_app_id) {
      $query->condition($or);
    }
  }

  // Group by.
  $query->groupBy('mediafile.' . mediamosa_asset_mediafile_db::APP_ID);

  // Get the data.
  $result = $query->execute();
  $num_rows = 0;
  foreach ($result as $record) {
    $num_rows++;
    if ($num_rows > DRUSH_STATISTICS_DISKUSAGE_MAXQUERY) {
      $drush_response->add_program_error(dt('More than @query application id counted. Stop now.', array('@query' => DRUSH_STATISTICS_DISKUSAGE_MAXQUERY)));
      break;
    }
    $data = array(
      'app_id' => $record[mediamosa_asset_mediafile_db::APP_ID],
      'diskusage' => $record['sum_filesize'],
      'quota' => $record[mediamosa_app_db::QUOTA],
    );
    $drush_response->add_item($data);
  }
}
