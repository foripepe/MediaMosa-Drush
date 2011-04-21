<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Constants.
 */
define('DRUSH_MEDIAMOSA_DURATION_LIMIT', 10);

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
function drush_mediamosa_mm_duration($subcommand = NULL) {

  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Verbal.
  $verbal = (drupal_strtolower(drush_get_option('verbal')) != 'false');

  // Limit.
  $limit = (int) drush_get_option('limit');
  if (!$limit) {
    $limit = DRUSH_MEDIAMOSA_DURATION_LIMIT;
  }

  // Run the tests.
  _drush_mediamosa_mm_duration_check($limit, $verbal);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_duration_validate() {
  // Limit.
  $limit = drush_get_option('limit');
  if ($limit && !is_numeric($limit)) {
    return drush_set_error('LIMIT_IS_NOT_NUMERIC', dt('The limit parameter is not numeric. See "drush help mm-duration".'));
  }

  // Verbal.
  $verbal = drush_get_option('verbal');
  if ($verbal && !in_array(drupal_strtolower($verbal), array('true', 'false'))) {
    return drush_set_error('VERBAL_IS_NOT_BOOLEAN', dt('The verbal parameter is not boolean. See "drush help mm-duration".'));
  }
}

/**
 * Check.
 * Show the mediafiles of MediaMosa, where the file duration of original and the transcoded mediafiles are very different.
 */
function _drush_mediamosa_mm_duration_check($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking file duration of transcoded mediafiles]'));
  }

  // Prop ids.
  $prop_id_file_duration = mediamosa_asset_mediafile_metadata_property::get_property_id_char('file_duration');

  // Query.

  timer_start('drush_mediamosa_duration');

  $result = mediamosa_db::db_query("
    SELECT
      org.#asset_id AS asset_id,
      org.#mediafile_id AS mediafile_id,
      (SUBSTR(meta.#val_char,1,2) * 3600) + (SUBSTR(meta.#val_char,4,2) * 60) + SUBSTR(meta.#val_char,7,2) AS org_sec,
      trans.#mediafile_id as trans_mediafile_id,
      (SUBSTR(meta2.#val_char,1,2) * 3600) + (SUBSTR(meta2.#val_char,4,2) * 60) + SUBSTR(meta2.#val_char,7,2) AS trans_sec

    FROM {#mediamosa_asset_mediafile} AS org
    JOIN {#mediamosa_asset_mediafile_metadata} AS meta ON (meta.#mediafile_id_metadata = org.#mediafile_id AND meta.#prop_id = :prop_id)
    JOIN {#mediamosa_asset_mediafile} AS trans ON (trans.#asset_id = org.#asset_id AND trans.#mediafile_id_source = org.#mediafile_id)
    LEFT JOIN {#mediamosa_asset_mediafile_metadata} AS meta2 ON (trans.#mediafile_id = meta2.#mediafile_id_metadata AND meta2.#prop_id = :prop_id)

    WHERE org.#is_original_file = :is_original_file_true AND trans.#is_original_file = :is_original_file_false AND org.#is_still = :is_still_false AND trans.#is_still = :is_still_false AND meta.#val_char IS NOT NULL
    HAVING trans_sec IS NULL OR ABS(org_sec - trans_sec) > (org_sec / 10)
    ORDER BY ABS(org_sec - trans_sec) DESC
  ", array(
    '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
    '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
    '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
    '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
    '#mediafile_id_metadata' => mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
    '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
    '#val_char' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
    '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
    '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
    '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
    ':is_original_file_true' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
    ':is_original_file_false' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE,
    ':is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
    ':prop_id' => $prop_id_file_duration,
  ));
  $row_count = $result->rowCount();

  $execution_time = timer_stop('drush_mediamosa_duration');

  if ($verbal) {
    drush_print(dt('[Execution time: @count ms]', array('@count' => $execution_time['time'])));
    drush_print(dt('Total: @count', array('@count' => $row_count)));
  }
  else {
    drush_print($row_count);
  }

  if ($verbal) {
    drush_print(dt('@count result(s):', array('@count' => min($limit, $row_count))));
    $i = 0;
    foreach ($result as $record) {
      drush_print(dt('@count) asset_id=@asset_id; mediafile_id(orig)=@mediafile_id_orig; sec(orig)=@sec_orig; mediafile_id(trans)=@mediafile_id_trans; sec(trans)=@sec_trans; @url', array(
        '@count' => ++$i,
        '@asset_id' => $record['asset_id'],
        '@mediafile_id_orig' => $record['mediafile_id'],
        '@sec_orig' => $record['org_sec'],
        '@mediafile_id_trans' => $record['trans_mediafile_id'],
        '@sec_trans' => $record['trans_sec'],
        '@url' => url('admin/mediamosa/browse/asset/' . $record['asset_id'], array('absolute' => TRUE)),
      )));

      if ($i >= $limit) {
        break;
      }
    }
  }
}
