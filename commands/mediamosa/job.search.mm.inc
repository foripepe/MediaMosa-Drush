<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Required files.
 */
require_once 'drush_mediamosa_library.inc';

/**
 * Constants.
 */
define('DRUSH_MEDIAMOSA_FIELD_SEPARATOR', ':');
define('DRUSH_MEDIAMOSA_FIELD_EQUAL', '=');
define('DRUSH_MEDIAMOSA_VALUE_SEPARATOR', ',');

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
function drush_mediamosa_mm_search_job($subcommand = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Get the options.

  // Fields.
  // We have "status=finished,waiting:job_type=analyse,transcode".
  // Explode to ":". Eg. "status=finished,waiting".
  $fields = explode(DRUSH_MEDIAMOSA_FIELD_SEPARATOR, drush_get_option('fields'));
  $fields_processed = array();
  foreach ($fields as $key => $field) {
    // Do we have fields? Eg. "status=finished,waiting".
    if ($field) {
      // Explode to "=". Eg."status" and "finished,waiting".
      $arg_value = explode(DRUSH_MEDIAMOSA_FIELD_EQUAL, $field);
      // It must be two parts.
      if (count($arg_value) == 2) {
        // $arg is eg. "status", and $value is eg. "finished,waiting".
        list($arg, $value) = explode(DRUSH_MEDIAMOSA_FIELD_EQUAL, $field);
        // Explode to ",". Eg. "finished" and "waiting".
        $fields_processed[check_plain($arg)] = explode(DRUSH_MEDIAMOSA_VALUE_SEPARATOR, check_plain($value));
        foreach ($fields_processed[check_plain($arg)] as $value_key => $value_value) {
          // Get the values.
          $fields_processed[check_plain($arg)][$value_key] = check_plain($value_value);
        }
      }
    }
  }

  // Start and end.
  $start = (drush_get_option('start') ? mediamosa_datetime::app_date2utc_date(drush_get_option('start') . drupal_substr('0000-01-01 00:00:00', drush_get_option('start')), 'Europe/Amsterdam') : NULL);
  $end = (drush_get_option('end') ? mediamosa_datetime::app_date2utc_date(drush_get_option('end') . drupal_substr('0000-01-01 00:00:00', drush_get_option('end')), 'Europe/Amsterdam') : NULL);

  // Limit.
  $limit = (int) drush_get_option('limit');

  // Order_by and order_direction.
  $order_by = check_plain(drush_get_option('order_by'));
  $order_direction = check_plain(drush_get_option('order_direction'));
  if ($order_by && !$order_direction) {
    $order_direction = 'asc';
  }

  // Output.
  $output = check_plain(drush_get_option('output'));

  // Verbal.
  $verbal = (drupal_strtolower(drush_get_option('verbal')) != 'false');

  _drush_mediamosa_mm_search_job_collect($fields_processed, $output, $start, $end, $limit, $order_by, $order_direction, $verbal);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_search_job_validate() {
  $schema = drupal_get_schema(mediamosa_job_db::TABLE_NAME);
  $schema_fields = $schema['fields'];

  // Get the options.

  // Fields.
  // We have "status=finished,waiting:job_type=analyse,transcode".
  // Explode to ":". Eg. "status=finished,waiting".
  $fields = explode(DRUSH_MEDIAMOSA_FIELD_SEPARATOR, drush_get_option('fields'));
  $fields_processed = array();
  foreach ($fields as $key => $field) {
    // Do we have fields? Eg. "status=finished,waiting".
    if ($field) {
      // Explode to "=". Eg."status" and "finished,waiting".
      $arg_value = explode(DRUSH_MEDIAMOSA_FIELD_EQUAL, $field);
      // It must be two parts.
      if (count($arg_value) != 2) {
        return drush_set_error('WRONG_ARGUMENT_NUMBER', dt('Wrong argument number: @arg_value. See "drush help mm-stats-tc".', array('@arg' => print_r($arg_value, TRUE))));
      }

      // $arg is eg. "status", and $value is eg. "finished,waiting".
      list($arg, $value) = explode(DRUSH_MEDIAMOSA_FIELD_EQUAL, $field);
      // Argumen (eg. "status") is (not) in the schema?
      if (!array_key_exists($arg, $schema_fields)) {
        return drush_set_error('WRONG_ARGUMENT', dt('Wrong argument: @arg. See "drush help mm-stats-tc".', array('@arg' => $arg)));
      }
      // Explode to ",". Eg. "finished" and "waiting".
      $fields_processed[check_plain($arg)] = explode(DRUSH_MEDIAMOSA_VALUE_SEPARATOR, check_plain($value));
      $type = $schema_fields[$arg]['type'];
      // Check the values if they are enum.
      if (drupal_substr($type, 0, strlen('enum')) == 'enum') {
        // Get the available enum fields (started 1 char after the "enum" text and end 1 char before the string end).
        $type = drupal_substr($type, strlen('enum') + 1 , -1);
        // Allowed type fields.
        $type = explode(',', $type);
        foreach ($fields_processed[check_plain($arg)] as $value_key => $value_value) {
          // $value_value is eg. "finished". $type has all the available enum fields.
          if (!in_array("'" . drupal_strtoupper($value_value) . "'", $type)) {
            return drush_set_error('WRONG_VALUE', dt('Wrong value: @arg = @value. See "drush help mm-stats-tc".', array('@arg' => $arg, '@value' => $value_value)));
          }
        }
      }
    }
  }

  // Start and end.
  $pattern = '/^(\d{4})(-(\d{2}(-(\d{2}( \d{2}(:\d{2}(:\d{2})?)?)?))?))?$/';
  $start = drush_get_option('start');
  if ($start) {
    if (!preg_match($pattern, $start, $matches)) {
      return drush_set_error('START_FORMAT_IS_WRONG', dt('The format of start option is wrong. See "drush help mm-search-job".'));
    }
    $start = drush_mediamosa_library_iso8601date_to_unix($start);
    if ($start === FALSE) {
      return drush_set_error('START_DATE_IS_INVALID', dt('The date of start option is invalid. See "drush help mm-search-job".'));
    }
  }
  $end = drush_get_option('end');
  if ($end) {
    if (!preg_match($pattern, $end, $matches)) {
      return drush_set_error('END_FORMAT_IS_WRONG', dt('The format of end option is wrong. See "drush help mm-search-job".'));
    }
    $end = drush_mediamosa_library_iso8601date_to_unix($end);
    if ($end === FALSE) {
      return drush_set_error('END_DATE_IS_INVALID', dt('The end of start option is invalid. See "drush help mm-search-job".'));
    }
  }
  if ($start && $end && $end < $start) {
    return drush_set_error('END_IS_EARLIER_THAN_START', dt('The end date is earlier, than the start. See "drush help mm-search-job".'));
  }

  // Limit.
  $limit = drush_get_option('limit');
  if ($limit && !is_numeric($limit)) {
    return drush_set_error('LIMIT_IS_NOT_NUMERIC', dt('The limit parameter is not numeric. See "drush help mm-search-job".'));
  }

  // Order_by and order_direction.
  $order_by = drush_get_option('order_by');
  $order_direction = drush_get_option('order_direction');
  if ($order_by) {
    if (!array_key_exists($order_by, $schema_fields)) {
      return drush_set_error('ORDER_BY_IS_INVALID', dt('The order_by parameter is not valid. See "drush help mm-search-job".'));
    }
  }
  if ($order_by && $order_direction && !in_array(drupal_strtolower($order_direction), array('asc', 'desc',))) {
    return drush_set_error('ORDER_DIRECTION_IS_INVALID', dt('The order_direction parameter is not "asc" or "desc". See "drush help mm-search-job".'));
  }

  // Output.
  if (drush_get_option('output')) {
    $output = explode(DRUSH_MEDIAMOSA_VALUE_SEPARATOR, drush_get_option('output'));
    foreach ($output as $column) {
      // Check the transcode_profile_id (= profile_id) too. It is not part of the mediamosa_job table.
      if (!array_key_exists($column, $schema_fields) && $column != 'transcode_profile_id' && $column != 'profile_id') {
        return drush_set_error('OUTPUT_IS_INVALID', dt('One of output parameter (@parameter) is not valid. See "drush help mm-search-job".', array('@parameter' => $column)));
      }
    }
  }

  // Verbal.
  $verbal = drush_get_option('verbal');
  if ($verbal && !in_array(drupal_strtolower($verbal), array('true', 'false'))) {
    return drush_set_error('VERBAL_IS_NOT_BOOLEAN', dt('The verbal parameter is not boolean. See "drush help mm-search-job".'));
  }

}

/**
 * Collect the information about the job table.
 */
function _drush_mediamosa_mm_search_job_collect($fields_processed, $output, $start, $end, $limit, $order_by, $order_direction, $verbal) {
  $type = '';
  foreach ($fields_processed as $column => $values) {
    $type .= $column . '=';
    foreach ($values as $value) {
      $type .= $value . ',';
    }
    $type .= ' ';
  }
  $type = drupal_substr($type, 0, -1);

  if ($verbal) {
    drush_print(dt('[Job search: @type]', array('@type' => $type,)));
  }

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'job');

  // Joins.
  $transcode = $query->leftjoin(mediamosa_job_transcode_db::TABLE_NAME, 'transcode', 'job.job_id = transcode.job_id');


  // Fields.
  $output_fields = explode(DRUSH_MEDIAMOSA_VALUE_SEPARATOR, $output);
  // Transcode_profile_id. We allowed "profile_id" and "transcode_profile_id" as a column name. It is "transcode_profile_id" in the db.
  $pos = array_search('profile_id', $output_fields);
  if (!$pos) {
    $pos = array_search('transcode_profile_id', $output_fields);
  }
  if ($pos !== FALSE) {
    // Unset transcode_profile_id from the mediamosa_job table.
    unset($output_fields[$pos]);
  }
  // Main fields.
  // array() means all fields.
  // $output_fields the user field.
  // array_unique(array_merge()) is user fields plus asset_id (it needs for the link created later).
  $query->fields('job', ($output ? array_unique(array_merge($output_fields, array(mediamosa_job_db::ASSET_ID))) : array()));
  // Transcode fields in the mediamosa_job_transcode table.
  if ($pos !== FALSE || !$output) {
    $query->fields('transcode', array('transcode_profile_id'));
  }


  // Condition for --fields.
  foreach ($fields_processed as $field => $values) {
    $or = db_or();
    foreach ($values as $value) {
      $or->condition('job.' . $field, $value);
    }
    $query->condition($or);
  }

  // Start and end.
  if ($start || $end) {
    $query->where('job.' . mediamosa_job_db::STARTED . ' <= job.' . mediamosa_job_db::FINISHED);
    if ($end) {
      $query->where('job.' . mediamosa_job_db::STARTED . ' >= :start', array(':start' => $start));
    }
    if ($start) {
      $query->where('job.' . mediamosa_job_db::FINISHED . ' <= :end', array(':end' => $end));
    }
  }

  // Order_by and order_direction.
  if ($order_by) {
    $query->orderBy($order_by, $order_direction);
  }

  // Count.
  try {
    $num_rows = $query->countQuery()->execute()->fetchField();
    if ($verbal) {
      drush_print(dt('Total number of your query is: @count', array('@count' => $num_rows)));
    }
    else {
      drush_print($num_rows);
    }

    // Do we have to show the rows?
    if ($num_rows > 0 && $limit > 0 && $verbal) {
      $result_num = min($num_rows, $limit);
      drush_print(format_plural($result_num, '1 result:', '@count results:'));
      // Range.
      $query->range(0, $limit);
      $result = $query->execute();
      $i = 0;
      foreach ($result as $record) {
        // $record is a row.
        $print_fields = '';
        foreach ($record as $column => $field) {
          // $column is the name of column, and $field is the value.

          if ($column == 'transcode_profile_id' && !$field) {
            // Skip the transcode_profile_id, if not exists.
            continue;
          }
          // Skip the asset_id, if it wasn't requested.
          if ($column == mediamosa_job_db::ASSET_ID && !in_array(mediamosa_job_db::ASSET_ID, $output_fields)) {
            continue;
          }
          // Generate output.
          $print_fields .= $column . '=' . $field . '; ';
        }
        drush_print(dt('@i) @fields@url', array('@i' => ++$i,'@fields' => $print_fields, '@url' => url('admin/mediamosa/browse/asset/' . $record[mediamosa_job_db::ASSET_ID], array('absolute' => TRUE)),)));
      }
    }
  }
  catch (PDOException $e) {
    drush_print(dt('Catch a PDO Exception. Check your --fields option. Error message: !message', array('!message' => $e->getMessage())));
  }
}
