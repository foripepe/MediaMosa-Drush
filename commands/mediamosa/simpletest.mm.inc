<?php
// $Id: $

/**
 * Copyright (c) 2011 Madcap BV (http://www.madcap.nl)
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
function drush_mediamosa_mm_simpletest() {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Analyse the mediafiles.
  drush_print(dt('[Starting to get information about simpletest.]'));
  _drush_mediamosa_mm_simpletest();
  drush_print(dt('[Simpletest is finished.]'));
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_simpletest_validate() {
}

/**
 * Analyse mediafiles.
 *
 * @param $csv
 * @param $mediafile_ids array
     A not empty array with mediafile ids.
 */
function _drush_mediamosa_mm_simpletest() {
  $response = drush_mediamosa_library_simpletest();
  drush_print(dt('Total number: @total', array('@total' => $response['header']['item_count_total'])));

  $total = 0;
  $success = 0;
  $failed = 0;
  $last_run_from = NULL;
  $last_run_to = NULL;
  foreach ($response['items'] as $simpletest) {
    $total++;
    if ($simpletest['fails'] || $simpletest['exceptions']) {
      $failed++;
    }
    else {
      $success++;
    }

    if (!$last_run_from || $simpletest['last_run'] < $last_run_from) {
      $last_run_from = $simpletest['last_run'];
    }
    if (!$last_run_to || $simpletest['last_run'] > $last_run_to) {
      $last_run_to = $simpletest['last_run'];
    }
  }

  drush_print(dt('Number of success runs: @success', array('@success' => $success)));
  drush_print(dt('Number of failed tests: @failed', array('@failed' => $failed)));
  drush_print(dt('The tests run from @from to @to', array('@from' => $last_run_from, '@to' => $last_run_to,)));
}
