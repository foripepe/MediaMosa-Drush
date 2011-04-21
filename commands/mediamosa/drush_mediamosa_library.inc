<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Constants.
 */
define('DRUSH_MEDIAMOSA_LIBRARY_MAX_JOB_SLOTS', 10);

/**
 * Transcode mediafile.
 *
 * @param $mediafile_id
 */
function drush_mediamosa_library_transcode($mediafile_id, $profile_id = NULL) {
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
  );

  // Getting the mediafile.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);
  if ($mediafile) {
    $uri = sprintf('mediafile/%s/transcode', $mediafile[mediamosa_asset_mediafile_db::ID]);
    $method = 'POST';
    $params = array(
      'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
    );
    if ($profile_id) {
      $params['profile_id'] = $profile_id;
    }
    $response = mediamosa_response_connector::static_do_restcall($uri, $method, $params);
    $job_id = (isset($response['items'][1]['job_id']) ? (string)$response['items'][1]['job_id'] : NULL);
    if ($job_id) {
      drush_print(dt('@id [ok, job_id=@job_id]', array(
        '@id' => $mediafile[mediamosa_asset_mediafile_db::ID],
        '@job_id' => $job_id,
      )));
    }
    else {
      drush_print(dt('@id [fail]', array('@id' => $mediafile[mediamosa_asset_mediafile_db::ID])));
    }
  }
  else {
    drush_print(dt('@id [fail : does not exist.]', array('@id' => $mediafile_id)));
  }
}

/**
 * Retranscode mediafile.
 *
 * @param $mediafile_id
 */
function drush_mediamosa_library_retranscode($mediafile_id, $profile_id = NULL) {
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
  );

  // Getting the mediafile.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);
  if (!$mediafile) {
    drush_print(dt('@id [fail : does not exist.]', array('@id' => $mediafile_id)));
    return;
  }

  if ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE) {
    drush_print(dt('@id [fail : original file (can not retranscode).]', array('@id' => $mediafile_id)));
    return;
  }

  $uri = sprintf('mediafile/%s/retranscode', $mediafile[mediamosa_asset_mediafile_db::ID]);
  $method = 'POST';
  $params = array(
    'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
    'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
  );
  if ($profile_id) {
    $params['profile_id'] = $profile_id;
  }
  $response = mediamosa_response_connector::static_do_restcall($uri, $method, $params);
  $job_id = (isset($response['items'][1]['job_id']) ? (string)$response['items'][1]['job_id'] : NULL);
  if (!$job_id) {
    drush_print(dt('@id [fail : job was not created]', array('@id' => $mediafile[mediamosa_asset_mediafile_db::ID])));
    return;
  }

  drush_print(dt('@id [ok, job_id=@job_id]', array(
    '@id' => $mediafile[mediamosa_asset_mediafile_db::ID],
    '@job_id' => $job_id,
  )));
}

/**
 * Analyse mediafile.
 *
 * @param $mediafile_id
 */
function drush_mediamosa_library_analyse($mediafile_id, $hint = NULL) {
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
  );

  // Getting the mediafile.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);

  if ($mediafile) {
    $uri = sprintf('mediafile/%s/analyse', $mediafile[mediamosa_asset_mediafile_db::ID]);
    $method = 'POST';
    $params = array(
      'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
    );

    if ($hint) {
      $params['hint'] = $hint;
    }

    $response = mediamosa_response_connector::static_do_restcall($uri, $method, $params);
    $job_id = (string)$response['items'][1]['job_id'];
    if ($job_id) {
      drush_print(dt('@id [ok, job_id=@job_id]', array(
        '@id' => $mediafile[mediamosa_asset_mediafile_db::ID],
        '@job_id' => $job_id,
      )));
    }
    else {
      drush_print(dt('@id [fail]', array('@id' => $mediafile[mediamosa_asset_mediafile_db::ID])));
    }
  }
  else {
    drush_print(dt('@id [fail]', array('@id' => $mediafile_id)));
  }
}

/**
 * Simpletest.
 *
 * @param $mediafile_id
 */
function drush_mediamosa_library_simpletest() {
  $uri = sprintf('simpletest');
  $method = 'GET';
  $params = array(
    mediamosa_settings::MEDIAMOSA_RUN => mediamosa_settings::MEDIAMOSA_RUN_ALL,
    mediamosa_rest_call_simpletest::SKIP_EMPTY => FALSE,
  );

  $response = mediamosa_response_connector::static_do_restcall($uri, $method, $params);

  return $response;
}

/**
 * Delete a single mediafile.
 *
 * @param $mediafile_id
 */
function drush_mediamosa_library_delete($mediafile_id) {
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
  );

  // Getting the mediafile.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);
  if ($mediafile) {
    $uri = sprintf('mediafile/%s/delete', $mediafile[mediamosa_asset_mediafile_db::ID]);
    $method = 'POST';
    $params = array(
      'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
    );
    $response = drush_mediamosa_execute_restcall($uri, $method, $params);
    $result_id = (string) $response->header->request_result_id;
    if ($result_id == mediamosa_sdk::ERRORCODE_OKAY) {
      drush_print(dt('@id [ok]', array(
        '@id' => $mediafile[mediamosa_asset_mediafile_db::ID],
      )));
    }
    else {
      drush_print(dt('@id [fail @error_id]', array(
        '@id' => $mediafile[mediamosa_asset_mediafile_db::ID],
        '@error_id' => $result_id,
      )));
    }
  }
  else {
    drush_print(dt('@id [fail : does not exist.]', array('@id' => $mediafile_id)));
  }
}

/**
 * Still generate for mediafile.
 *
 * @param string $mediafile_id
 * @param int $frametime
 * @param string $size
 *   Size, like 120x100.
 */
function drush_mediamosa_library_still($mediafile_id, $frametime = NULL, $size = NULL) {
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::APP_ID,
    mediamosa_asset_mediafile_db::OWNER_ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  );

  // Getting the mediafile.
  $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, $fields);
  if (!$mediafile) {
    drush_print(dt('@id [fail : does not exist.]', array('@id' => $mediafile_id)));
    return;
  }

  // Check if the mediafile has <= 1 still.
  $stills_num = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
    ->condition(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $mediafile_id)
    ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE)
    ->countQuery()->execute()->fetchField();
  if ($stills_num > 1) {
    drush_print(dt('Still generation skipped, more, then 1 still (@stills_num): @id [ok]', array(
      '@id' => $mediafile_id,
      '@stills_num' => $stills_num,
    )));
    return;
  }

  // Delete the existing still.
  $uri = sprintf('asset/%s/still/delete', $mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);
  $method = 'POST';
  $params = array(
    'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
    'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
    'mediafile_id' => $mediafile_id,
  );
  $response = drush_mediamosa_execute_restcall($uri, $method, $params);
  $result_id = (string) $response->header->request_result_id;
  if ($result_id == mediamosa_sdk::ERRORCODE_OKAY) {
    drush_print(dt('Still delete: @id [ok]', array(
      '@id' => $mediafile_id,
    )));
  }
  else {
    drush_print(dt('Still delete: @id [fail @error_id]', array(
      '@id' => $mediafile_id,
      '@error_id' => $result_id,
    )));
  }

  // Create new still.
  $uri = sprintf('mediafile/%s/still/create', $mediafile_id);
  $method = 'POST';
  $params = array(
    'app_id' => $mediafile[mediamosa_asset_mediafile_db::APP_ID],
    'user_id' => $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
  );
  if ($frametime) {
    $params['frametime'] = $frametime;
  }
  if ($size) {
    $params['size'] = $size;
  }
  $response = drush_mediamosa_execute_restcall($uri, $method, $params);
  $result_id = (string) $response->header->request_result_id;
  if ($result_id == mediamosa_sdk::ERRORCODE_OKAY) {
    drush_print(dt('Still create: @id [ok]', array(
      '@id' => $mediafile_id,
    )));
  }
  else {
    drush_print(dt('Still create: @id [fail @error_id]', array(
      '@id' => $mediafile_id,
      '@error_id' => $result_id,
    )));
  }
}

/**
 * HTTP connections.
 * Then the user will be www-data, and not the user, who started the drush command.
 */
function drush_mediamosa_execute_restcall($orig_url, $method = 'GET', $data = array()) {
  // mediamosa_cron_url_app
  // mediamosa_internal_password
  // mediamosa_connector_username
  // mediamosa_connector_password
  // mediamosa_connector_url
    // URI.
    $connector_uri = variable_get('mediamosa_connector_url', NULL);
    if (drupal_substr($connector_uri, -1) != '/') {
      $connector_uri .= '/';
    }

    // Use the url
    $url = $connector_uri . 'internal/' . $orig_url;

    // Secret password for internal usage.
    $options = array();
    $options['headers']['X-Mediamosa-Internal-Pass'] = variable_get('mediamosa_internal_password', NULL);
    $options['method'] = $method;
    //$options['data'] = http_build_query($data);
    $url .= '?' . http_build_query($data);

    $response = drupal_http_request($url, $options);

    if (isset($response->error) || ($response->code && $response->code != 200)) {
      drush_print(dt('Error: [@code] @error, Url: @url', array(
        '@code' => $response->code,
        '@error' => $response->error,
        '@url' => $url,
      )));
      die;
    }

    $xml = new SimpleXMLElement($response->data);

    return $xml;
}

/**
 * Trim the values in an array.
 *
 * @param $item
 * @param $key
 */
function drush_mediamosa_library_array_trim(&$item, $key) {
  $item = trim($item);
}

/**
 * Clean the empty values in an array.
 *
 * @param $value
 *   A value of the array.
 */
function drush_mediamosa_library_array_clean_empty($value) {
  return (bool)$value;
}

/**
 * Gets the number of the waiting jobs.
 *
 * @return the number of the waiting jobs
 */
function drush_mediamosa_library_get_waiting_jobs() {
  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'job');

  // Condition.
  $query
    ->condition('job.' . mediamosa_job_db::JOB_STATUS, mediamosa_job_db::JOB_STATUS_WAITING);

  $waiting_jobs = $query->countQuery()->execute()->fetchField();

  return $waiting_jobs;
}

/**
 * Gets the number of the available job slots.
 *
 * @return the number of the available job slots
 */
function drush_mediamosa_library_get_available_job_slots() {
  $slots = max(0, DRUSH_MEDIAMOSA_LIBRARY_MAX_JOB_SLOTS - drush_mediamosa_library_get_waiting_jobs());
  return $slots;
}

/**
 * Open and check the file with ids.
 *
 * @param $file
 *
 * @return $ids
 */
function drush_mediamosa_library_file_open_check($file) {
  // Get the  CSV file.
  $ids = file($file);

  // Check.
  if (!$ids || !is_array($ids) || empty($ids)) {
    drush_print(dt('[Error opening the CSV file (@file), or the CSV file is empty.]', array('@file' => $file)));
    return;
  }
  // Clean up.
  array_walk($ids, 'drush_mediamosa_library_array_trim');
  $ids = array_filter($ids, 'drush_mediamosa_library_array_clean_empty');
  // Check again.
  if (empty($ids)) {
    drush_print(dt("[The CSV file (@file) doesn't have ids.]", array('@file' => $file)));
    return;
  }

  return $ids;
}

/**
 * Save the file with ids.
 *
 * @param $file
 * @param $ids
 */
function drush_mediamosa_library_file_save($file, $ids) {
  if (!$ids) {
    unlink($file);
  }
  else {
    if (!$handle = fopen($file, 'w')) {
      drush_print(dt("[Error opening the CSV file (@file) to write.]", array('@file' => $file)));
      return;
    }
    if (fwrite($handle, implode("\n", $ids)) === FALSE) {
      drush_print(dt("[Error writing the CSV file (@file).]", array('@file' => $file)));
      return;
    }
    fclose($handle);
  }
}

/**
 * Convert YYYY-MM-DD HH:MM:SS to number of seconds since Jan. 1st 1970.
 *
 * Takes date as it is, does not convert to any timezone.
 *
 * @param string $iso8601_date_str
 */
function drush_mediamosa_library_iso8601date_to_unix($iso8601_date_str) {

  $iso8601_date_str = $iso8601_date_str . drupal_substr('0000-01-01 00:00:00', drupal_strlen($iso8601_date_str));

  // Parse the date.
  $date = date_parse($iso8601_date_str);

  // Get current timezone
  $tz = date_default_timezone_get();

  // Set to GMT/UTC so we can convert without timezone problems (take as-is).
  date_default_timezone_set('UTC');

  // Convert to integer.
  $time = gmmktime($date['hour'], $date['minute'], $date['second'], $date['month'], $date['day'], $date['year']);

  // Back to original timezone, so we dont screw with other time functions
  date_default_timezone_set($tz);

  return $time;
}
