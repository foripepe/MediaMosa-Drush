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
require_once 'drush_mediamosa_library.inc';

/**
 * Constants.
 */
define('DRUSH_STATISTICS_STREAMING_MAXQUERY', 1000);

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
function drush_mediamosa_mm_stats_st($subcommand = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Get the options.
  $app_ids = explode(',', drush_get_option('app_ids'));
  $start_timestamp = drush_mediamosa_library_iso8601date_to_unix(drush_get_option('start'));
  $end_timestamp = drush_mediamosa_library_iso8601date_to_unix(drush_get_option('end'));

  $drush_response = new drush_response();

  // Collect the information.
  _drush_mediamosa_mm_stats_st_collect($drush_response, $start_timestamp, $end_timestamp, $app_ids);

  // Return the information.
  $output = $drush_response->generate_xml();
  drush_print($output);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_stats_st_validate() {
  $start = drush_get_option('start');
  if (!$start) {
    return drush_set_error('START_IS_MISSING', dt('The start option is missing. See "drush help mm-stats-tc".'));
  }
  $end = drush_get_option('end');
  if (!$end) {
    return drush_set_error('END_IS_MISSING', dt('The end option is missing. See "drush help mm-stats-tc".'));
  }
  $pattern = '/^(\d{4})(-(\d{2}(-(\d{2}))?))?$/';
  if (!preg_match($pattern, $start, $matches)) {
    return drush_set_error('START_FORMAT_IS_WRONG', dt('The format of start option is wrong. See "drush help mm-stats-tc".'));
  }
  $pattern = '/^(\d{4})(-(\d{2}(-(\d{2}))?))?$/';
  if (!preg_match($pattern, $end, $matches)) {
    return drush_set_error('END_FORMAT_IS_WRONG', dt('The format of end option is wrong. See "drush help mm-stats-tc".'));
  }
  $start = drush_mediamosa_library_iso8601date_to_unix($start);
  if ($start === FALSE) {
    return drush_set_error('START_DATE_IS_INVALID', dt('The date of start option is invalid. See "drush help mm-stats-tc".'));
  }
  $end = drush_mediamosa_library_iso8601date_to_unix($end);
  if ($end === FALSE) {
    return drush_set_error('END_DATE_IS_INVALID', dt('The end of start option is invalid. See "drush help mm-stats-tc".'));
  }
  if ($end < $start) {
    return drush_set_error('END_IS_EARLIER_THAN_START', dt('The end date is earlier, than the start. See "drush help mm-stats-tc".'));
  }
}

/**
 * Collcet the transcoding information.
 *
 * @param $drush_output
 *   The output class.
 * @param $start_timestamp
 * @param $end_timestamp
 * @param $app_ids
 *   Application ids.
 */
function _drush_mediamosa_mm_stats_st_collect($drush_response, $start_timestamp, $end_timestamp, $app_ids = NULL) {
  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_statistics_stream_request_db::TABLE_NAME, 'st');

  // Expressions.
  $query->addExpression('COUNT(st.' . mediamosa_statistics_stream_request_db::APP_ID . ')', 'count');
  $query->addExpression('SUM(st.' . mediamosa_statistics_stream_request_db::BYTES . ')', 'bytes');

  // Fields.
  $query->fields('st', array(
    mediamosa_statistics_stream_request_db::APP_ID,
  ));

  // Condition.
  $query
    ->where('UNIX_TIMESTAMP(st.' . mediamosa_statistics_stream_request_db::TIME . ') <= :end', array(':end' => $end_timestamp))
    ->where('UNIX_TIMESTAMP(st.' . mediamosa_statistics_stream_request_db::TIME . ') >= :start', array(':start' => $start_timestamp));

  // Condition for application ids.
  if ($app_ids && is_array($app_ids) && !empty($app_ids)) {
    $or = db_or();
    $is_app_id = FALSE;
    foreach ($app_ids as $app_id) {
      if (is_numeric($app_id)) {
        $or->condition('st.' . mediamosa_statistics_stream_request_db::APP_ID, $app_id);
        $is_app_id = TRUE;
      }
    }
    if ($is_app_id) {
      $query->condition($or);
    }
  }

  // Group by.
  $query->groupBy('st.' . mediamosa_statistics_stream_request_db::APP_ID);

  // Get the data.
  $result = $query->execute();
  $num_rows = 0;
  foreach ($result as $record) {
    $num_rows++;
    if ($num_rows > DRUSH_STATISTICS_STREAMING_MAXQUERY) {
      $drush_response->add_program_error(dt('More than @query application id counted. Stop now.', array('@query' => DRUSH_STATISTICS_STREAMING_MAXQUERY)));
      break;
    }
    $data = array(
      'app_id' => $record[mediamosa_statistics_stream_request_db::APP_ID],
      'count' => $record['count'],
      'bytes' => $record['bytes'],
    );
    $drush_response->add_item($data);
  }
}
