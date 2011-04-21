<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Required files.
 */
require_once 'du.stats.mm.inc';
require_once 'tc.stats.mm.inc';
require_once 'st.stats.mm.inc';

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
function drush_mediamosa_mm_stats($subcommand = NULL) {
  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  drush_mediamosa_mm_stats_du($subcommand);
  drush_mediamosa_mm_stats_tc($subcommand);
  drush_mediamosa_mm_stats_st($subcommand);
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_mm_stats_validate() {
  $start = drush_get_option('start');
  if (!$start) {
    return drush_set_error('START_IS_MISSING', dt('The start option is missing. See "drush help mm-stats".'));
  }
  $end = drush_get_option('end');
  if (!$end) {
    return drush_set_error('END_IS_MISSING', dt('The end option is missing. See "drush help mm-stats".'));
  }
  $pattern = '/^(\d{4})(-(\d{2}(-(\d{2}))?))?$/';
  if (!preg_match($pattern, $start, $matches)) {
    return drush_set_error('START_FORMAT_IS_WRONG', dt('The format of start option is wrong. See "drush help mm-stats".'));
  }
  $pattern = '/^(\d{4})(-(\d{2}(-(\d{2}))?))?$/';
  if (!preg_match($pattern, $end, $matches)) {
    return drush_set_error('END_FORMAT_IS_WRONG', dt('The format of end option is wrong. See "drush help mm-stats".'));
  }
  $start = drush_mediamosa_library_iso8601date_to_unix($start);
  if ($start === FALSE) {
    return drush_set_error('START_DATE_IS_INVALID', dt('The date of start option is invalid. See "drush help mm-stats".'));
  }
  $end = drush_mediamosa_library_iso8601date_to_unix($end);
  if ($end === FALSE) {
    return drush_set_error('END_DATE_IS_INVALID', dt('The end of start option is invalid. See "drush help mm-stats".'));
  }
  if ($end < $start) {
    return drush_set_error('END_IS_EARLIER_THAN_START', dt('The end date is earlier, than the start. See "drush help mm-stats".'));
  }
}
