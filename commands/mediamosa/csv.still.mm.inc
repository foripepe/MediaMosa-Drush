<?php
// $Id: $

/**
 * Copyright (c) 2010 Madcap BV (http://www.madcap.nl)
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
function drush_mediamosa_mm_still_csv($csv = NULL, $frametime = NULL, $size = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Get the options.
  $mediafile_ids = drush_mediamosa_library_file_open_check($csv);
  if (!$mediafile_ids) {
    return;
  }

  // Transcode the mediafiles.
  drush_print(dt('[Starting to still generation the mediafiles.]'));
  _drush_mediamosa_mm_still_csv_mediafile($csv, $mediafile_ids, $frametime, $size);
  drush_print(dt('[Still generation for mediafiles has finished.]'));
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_still_csv_validate($csv = NULL, $frametime = NULL, $size = NULL) {
  if (!$csv) {
    return drush_set_error('PARAMETER_MISSING', dt('Parameter is missing. See "drush help mm-still-csv".'));
  }

  // Validate CSV file.
  if (!file_exists($csv)) {
    return drush_set_error('INPUT_FILE_NOT_EXISTS', dt('The CSV input file (@file) is not exists. See "drush help mm-still-csv".', array('@file' => $csv)));
  }
  if (!is_readable($csv)) {
    return drush_set_error('INPUT_FILE_NOT_READABLE', dt('The CSV input file (@file) is not readable. See "drush help mm-still-csv".', array('@file' => $csv)));
  }
  if (!is_writable($csv)) {
    return drush_set_error('INPUT_FILE_NOT_WRITABLE', dt('The CSV input file (@file) is not writable. See "drush help mm-still-csv".', array('@file' => $csv)));
  }

  // Frametime check.
  if ($frametime && (!is_numeric($frametime) || $frametime < 0)) {
    return drush_set_error('FRAMETIME_VALIDATION_FAILS', dt('The frametime (@frametime) is not numeric. See "drush help mm-still-csv".', array('@frametime' => $frametime)));
  }

  // TODO: size check.
}

/**
 * Generate stills for mediafiles.
 *
 * @param $csv
 * @param $mediafile_ids array
     A not empty array with mediafile ids.
 */
function _drush_mediamosa_mm_still_csv_mediafile($csv, $mediafile_ids, $frametime = NULL, $size = NULL) {
  try {
    foreach ($mediafile_ids as $key => $mediafile_id) {
      if (drush_mediamosa_library_get_available_job_slots() > 0) {
        drush_mediamosa_library_still($mediafile_id, $frametime, $size);
        unset($mediafile_ids[$key]);
      }
      else {
        drush_print(dt("[All job slots are occupied. Try again later. @count mediafile remains.]", array('@count' => count($mediafile_ids))));
        break;
      }
    }
  }
  catch (Exception $e) {
    if ($csv) {
      drush_mediamosa_library_file_save($csv, $mediafile_ids);
    }
    throw $e;
  }
  if ($csv) {
    drush_mediamosa_library_file_save($csv, $mediafile_ids);
  }
}