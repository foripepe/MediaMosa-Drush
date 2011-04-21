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
function drush_mediamosa_mm_analyse($mediafile_ids = NULL, $hint = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Get the options.
  $mediafile_ids = explode(',', $mediafile_ids);

  // Analyse the mediafiles.
  drush_print(dt('[Starting to analyse the mediafiles.]'));
  _drush_mediamosa_mm_analyse_mediafile($mediafile_ids, $hint);
  drush_print(dt('[Analyse the mediafiles is finished.]'));
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_analyse_validate($mediafile_ids = NULL, $hint = NULL) {
  if (!$mediafile_ids) {
    return drush_set_error('PARAMETER_MISSING', dt('Mediafile id parameter is missing. See "drush help mm-analyse".'));
  }
  if ($hint) {
    if (!in_array(drupal_strtoupper($hint), array('TRUE', 'FALSE'))) {
      return drush_set_error('HINT_PARAMETER_IS_NOT_VALID', dt('Hint parameter is not valid. See "drush help mm-analyse".'));
    }
  }
}

/**
 * Analyse mediafiles.
 *
 * @param $csv
 * @param $mediafile_ids array
     A not empty array with mediafile ids.
 */
function _drush_mediamosa_mm_analyse_mediafile($mediafile_ids, $hint) {
  foreach ($mediafile_ids as $key => $mediafile_id) {
    if (drush_mediamosa_library_get_available_job_slots() > 0) {
      drush_mediamosa_library_analyse($mediafile_id, $hint);
      unset($mediafile_ids[$key]);
    }
    else {
      drush_print(dt("[All job slots are occupied. Try again later. @count mediafile remains.]", array('@count' => count($mediafile_ids))));
      break;
    }
  }
}
