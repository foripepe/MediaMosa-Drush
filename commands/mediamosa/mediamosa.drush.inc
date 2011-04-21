<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * @file
 *   Mediamosa command.
 *
 *   To run this command, execute `drush mediamosa`
 *   from within your drush directory, or you can run it from anywhere,
 *   if you have aliases like `drush @vpcore mediamosa`.
 *
 *   You can copy this file to any of the following
 *     1. A .drush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/drush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your drush module makes available,
 * what it does and description.
 *
 * Notice how this structure closely resembles how you define menu hooks.
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function mediamosa_drush_command() {
  $items = array();

  $items['mm-monitor'] = array(
    'description' => 'Run tests for monitoring purposes.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--limit' => 'Limit of the output. Without limit you see 10 of results only. Eg. --limit=10',
      '--verbal' => 'If FALSE, then it gives back the mediafile ids only of mime type check (without the result of h264 video codec check). Default TRUE.',
      //'non' => 'Not yet implemented.',
    ),
    'examples' => array(
      'drush mm-monitor' => 'Runs the monitoring script once and outputs its findings.',
    ),
    'aliases' => array('mm-mon'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-stats-du'] = array(
    'description' => 'Show disk usage statistics.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--app_ids' => 'Optional. Comma delimited list of application ids (e.g. --app_ids=5,14).',
    ),
    'examples' => array(
      'drush mm-stats-du' => 'Gets disk usage and outputs its findings.',
      'drush mm-stats-du --app_ids=14' => 'Gets disk usage of application id=14 and outputs its findings.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-stats-st'] = array(
    'description' => 'Show streaming usage statistics.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--app_ids' => 'Optional. Comma delimited list of application ids (e.g. --app_ids=5,14).',
      '--start' => 'Required. Start date in YYYY[-MM[-DD]] format (e.g. --start=2008 or --start=2009-10). These two formats are equivalent --start=2006, --start=2006-01-01.',
      '--end' => 'Required. End date in YYYY[-MM[-DD]] format.',
    ),
    'examples' => array(
      'drush mm-stats-st --start=2006 --end=2007' => 'Gets streaming usage from 2006-01-01 to 2007-01-01 and outputs its findings.',
      'drush mm-stats-st --start=2006-03-22 --end=2007-10 --app_ids=14' => 'Gets streaming usage in the given interval of application id=14 and outputs its findings.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-stats-tc'] = array(
    'description' => 'Show transcode usage statistics.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--app_ids' => 'Optional. Comma delimited list of application ids (e.g. --app_ids=5,14).',
      '--start' => 'Required. Start date in YYYY[-MM[-DD]] format (e.g. --start=2008 or --start=2009-10). These two formats are equivalent --start=2006, --start=2006-01-01.',
      '--end' => 'Required. End date in YYYY[-MM[-DD]] format.',
    ),
    'examples' => array(
      'drush mm-stats-tc --start=2006 --end=2007' => 'Gets transcode usage from 2006-01-01 to 2007-01-01 and outputs its findings.',
      'drush mm-stats-tc --start=2006-03-22 --end=2007-10 --app_ids=14' => 'Gets transcode usage in the given interval of application id=14 and outputs its findings.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-stats'] = array(
    'description' => 'Show statistics.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--app_ids' => 'Optional. Comma delimited list of application ids (e.g. --app_ids=5,14).',
      '--start' => 'Required. Start date in YYYY[-MM[-DD]] format (e.g. --start=2008 or --start=2009-10). These two formats are equivalent --start=2006, --start=2006-01-01.',
      '--end' => 'Required. End date in YYYY[-MM[-DD]] format.',
    ),
    'examples' => array(
      'drush mm-stats --start=2006 --end=2007' => 'Gets disk usage and transcode usage from 2006-01-01 to 2007-01-01 and outputs its findings.',
      'drush mm-stats --start=2006-03-22 --end=2007-10 --app_ids=14' => 'Gets disk usage and transcode usage in the given interval of application id=14 and outputs its findings.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-transcode'] = array(
    'description' => 'Transcode one or more mediafiles.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id.',
      'profile_id' => 'Profile id.',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-transcode ALZbgPascjFtT9OLa8g7HtGp' => 'Transcode the mediafile, where the id is ALZbgPascjFtT9OLa8g7HtGp.',
    ),
    'aliases' => array('mm-tc'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-transcode-csv'] = array(
    'description' => 'Transcode one or more mediafiles, take a file with mediafile_id\' as input.',
    'arguments' => array(
      'input_csv' => 'The input CSV file.',
      'profile_id' => 'Profile id.',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-transcode-csv input.csv' => 'Transcode the mediafiles, where the ids are in the input.csv file.',
    ),
    'aliases' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-retranscode'] = array(
    'description' => 'Retranscode one or more mediafiles.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id.',
      'profile_id' => 'Profile id.',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-retranscode ALZbgPascjFtT9OLa8g7HtGp' => 'Retranscode the mediafile, where the id is ALZbgPascjFtT9OLa8g7HtGp.',
    ),
    'aliases' => array('mm-rtc'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-retranscode-csv'] = array(
    'description' => 'Retranscode one or more mediafiles, take a file with mediafile_id\' as input.',
    'arguments' => array(
      'input_csv' => 'The input CSV file.',
      'profile_id' => 'Profile id.',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-retranscode-csv input.csv' => 'Retranscode the mediafiles, where the ids are in the input.csv file.',
    ),
    'aliases' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-analyse'] = array(
    'description' => 'Analyse one or more mediafiles.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id.',
      'hint' => 'Hinting (TRUE, FALSE)',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-analyse ALZbgPascjFtT9OLa8g7HtGp' => 'Analyse the mediafile, where the id is ALZbgPascjFtT9OLa8g7HtGp.',
    ),
    'aliases' => array('mm-a'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-analyse-csv'] = array(
    'description' => 'Analyse one or more mediafiles, take a file with mediafile_id\'s as input.',
    'arguments' => array(
      'input_csv' => 'The input CSV file.',
      'hint' => 'Hinting (TRUE, FALSE)',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-analyse-csv input.csv' => 'Analyse the mediafile, where the ids are in the input.csv file.',
    ),
    'aliases' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-delete'] = array(
    'description' => 'Delete one or more mediafiles.',
    'arguments' => array(
      'mediafile_id' => 'Mediafile id.',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-delete ALZbgPascjFtT9OLa8g7HtGp' => 'Delete the mediafile with id ALZbgPascjFtT9OLa8g7HtGp.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-version'] = array(
    'description' => 'Show MediaMosa version number.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      //'--sg' => 'Not yet implemented.',
    ),
    'examples' => array(
      'drush mm-version' => 'Gives the version of MediaMosa. Eg. "2.3.1"',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-search-job'] = array(
    'description' => 'Searching jobs.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--fields' => 'List of parameters you want to search. Every parameter may have multiple values. Eg. --fields=status=finished,waiting:job_type=analyse,transcode - In this case the command gives back the number of jobs, where status is finished or waiting and job_type is analyse or transcode.',
      '--start' => 'Start date in YYYY[-MM[-DD[ HH[:MM[:SS]]]]] format (e.g. --start=2008 or --start=2009-10). These two formats are equivalent --start=2006, --start=2006-01-01.',
      '--end' => 'End date in YYYY[-MM[-DD[ HH[:MM[:SS]]]]] format.',
      '--output' => 'Comma delimited list of output arguments. Leave it empty, if you want to get all the fields (see --limit parameter too). All output field must be in the mediamosa_job table, except the "profile_id" (mediamosa_job_transcode field). Eg. --output=asset_id,app_id',
      '--limit' => 'Limit of the output. Without limit you see the number of results only. Eg. --limit=10',
      '--order_by' => 'On what column the result set should be ordered. Eg. --order_by=job_type',
      '--order_direction' => 'The direction of the result set (asc or desc; default: asc). Eg. --order_direction=desc',
      '--verbal' => 'If FALSE, then it gives back the number of results only. Default TRUE.',
    ),
    'examples' => array(
      'drush mm-search-job --fields= status=failed:job_type=transcode' => 'Outputs the number of failed transcode job.',
    ),
    'aliases' => array('mm-sj'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-slots'] = array(
    'description' => 'Show the used job slots of MediaMosa.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--verbal' => 'If FALSE, then it gives back the number of used, available and total slots (SPACE separated). Default TRUE.',
    ),
    'examples' => array(
      'drush mm-slots --verbal=FALSE' => 'Gives the numbers about the job slots of MediaMosa.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-duration'] = array(
    'description' => 'Show the mediafiles of MediaMosa, where the file duration of original and the transcoded mediafiles are very different.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      '--limit' => 'Limit of the output. Without limit you see 10 of results only. Eg. --limit=10',
      '--verbal' => 'If FALSE, then it gives back the number of result only. Default TRUE.',
    ),
    'examples' => array(
      'drush mm-duration --verbal=FALSE' => 'Gives the numbers of result, where the file duration of original and transcoded mediafiles are very different.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-mediafile-collect'] = array(
    'description' => 'Collect all the mediafiles and put them in the standard output. One mediafile_id per line.',
    'arguments' => array(
      //'subcommand' => 'Subcommand. Not yet implemented.',
    ),
    'options' => array(
      //'--limit' => 'Limit of the output. Without limit you see 10 of results only. Eg. --limit=10',
    ),
    'examples' => array(
      'drush mm-mc > mediafiles.txt' => 'Put all the mediafile_id in the mediafiles.txt file.',
    ),
    'aliases' => array('mm-mc'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-simpletest'] = array(
    'description' => 'Gives information about the status of simpletests.',
    'arguments' => array(
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-simpletest' => 'Get and output the information about the simpletests.',
    ),
    'aliases' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['mm-still-csv'] = array(
    'description' => 'Recreate the stills from a list of mediafiles in the given inputfile
      If the mediafile has more than 1 still, it does not do anything.
      If the mediafile has one still, it deletes that, and create a new one.
      If the still has not got still, then just create one.',
    'arguments' => array(
      'input_csv' => 'The input CSV file.',
      'frametime' => 'Still frametime (in seconds).',
      'size' => 'Size of the still (eg. 352x288).',
    ),
    'options' => array(
    ),
    'examples' => array(
      'drush mm-still-csv input.csv' => 'Create still for the mediafiles, where the ids are in the input.csv file.',
    ),
    'aliases' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}
