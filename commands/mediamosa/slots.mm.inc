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
function drush_mediamosa_mm_slots($subcommand = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Verbal.
  $verbal = (drupal_strtolower(drush_get_option('verbal')) != 'false');

  // Get the data.
  _drush_mediamosa_mm_slots($verbal);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_slots_validate() {
  // Verbal.
  $verbal = drush_get_option('verbal');
  if ($verbal && !in_array(drupal_strtolower($verbal), array('true', 'false'))) {
    return drush_set_error('VERBAL_IS_NOT_BOOLEAN', dt('The verbal parameter is not boolean. See "drush help mm-search-job".'));
  }
}

/**
 * Collect the information.
 */
function _drush_mediamosa_mm_slots($verbal) {
  // Get for every active server (status ON)
  // the number of job that is not FINISHED, FAILED of CANCELLED
  // and job type is not UPLOAD (does not take a slot).
  $result = mediamosa_db::db_query(
    "SELECT #nid, #uri, #slots,
           (SELECT COUNT(msj.#job_id)
            FROM {#mediamosa_server_job} AS msj
            JOIN {#mediamosa_job} AS mj ON mj.#job_id = msj.#job_id
            WHERE
              msj.#server_id = ms.#nid
              AND mj.#job_type != :JOBTYPE_UPLOAD
              AND mj.#job_status NOT IN (:JOBSTATUS_FINISHED, :JOBSTATUS_FAILED, :JOBSTATUS_CANCELLED)) AS jobcount
    FROM {#mediamosa_server} AS ms
    WHERE #server_status = :server_status_on AND #server_type = :server_type",
    array(
      '#nid' => mediamosa_server_db::NID,
      '#server_id' => mediamosa_server_job_db::SERVER_ID,
      '#uri' => mediamosa_server_db::SERVER_URI,
      '#slots' => mediamosa_server_db::SLOTS,
      '#job_type' => mediamosa_job_db::JOB_TYPE,
      '#job_id' => mediamosa_job_db::ID,
      '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
      '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
      '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
      ':JOBTYPE_UPLOAD' => mediamosa_job_db::JOB_TYPE_UPLOAD,
      ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
      ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      ':JOBSTATUS_CANCELLED' => mediamosa_job_db::JOB_STATUS_CANCELLED,
      '#job_status' => mediamosa_job_db::JOB_STATUS,
      '#server_status' => mediamosa_server_db::SERVER_STATUS,
      ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
      '#server_type' => mediamosa_server_db::SERVER_TYPE,
      ':server_type' => mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR,
    )
  )->fetchAll();

  $output = '';
  $slots_used = 0;
  $slots_available = 0;

  if ($verbal) {
    $output .= dt('[Slots]') . "\n";
  }

  foreach ($result as $result_row) {
    $slots_used += $result_row['jobcount'];
    $slots_available += $result_row['slots'];
    if ($verbal) {
      $output .= dt('  [Slot]') . "\n";
      $output .= dt('    [Nid: @nid]', array('@nid' => $result_row['nid'],)) . "\n";
      $output .= dt('    [Server URI: @server_uri]', array('@server_uri' => $result_row['server_uri'],)) . "\n";
      $output .= dt('    [Slots: @slots]', array('@slots' => $result_row['slots'],)) . "\n";
      $output .= dt('    [Jobcount: @jobcount]', array('@jobcount' => $result_row['jobcount'],)) . "\n";
    }
  }

  if ($verbal) {
    $output .= dt('[Used slots: @slots_used]', array('@slots_used' => $slots_used,)) . "\n";
    $output .= dt('[Available slots: @slots_available]', array('@slots_available' => $slots_available,)) . "\n";
    $output .= dt('[Slots total: @slots_total]', array('@slots_total' => ($slots_used + $slots_available),)) . "\n";
  }
  else {
    // Format: "USED AVAILABLE TOTAL".
    $output .= $slots_used . ' ' . $slots_available . ' ' . ($slots_used + $slots_available);
  }

  drush_print($output);
}
