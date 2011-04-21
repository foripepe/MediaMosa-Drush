<?php
// $Id: $

/**
 * Copyright (c) 2009 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 */

/**
 * Constants.
 */
define('DRUSH_MEDIAMOSA_MONITOR_LIMIT', 10);

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
function drush_mediamosa_mm_monitor($subcommand = NULL) {

  // Simple check if we are running Drupal 7.
  if (drush_drupal_major_version() < 7) {
    return drush_set_error('Requires Drupal 7 or higher installation.');
  }

  // Verbal.
  $verbal = (drupal_strtolower(drush_get_option('verbal')) != 'false');

  // Limit.
  $limit = (int) drush_get_option('limit');
  if (!$limit) {
    $limit = DRUSH_MEDIAMOSA_MONITOR_LIMIT;
  }

  // Header.
  if ($verbal) {
    drush_print(dt('[Result daily monitoring]'));
  }

  // Run the tests.
  _drush_mediamosa_check_mime_type($limit, $verbal);
  if ($verbal) {
    _drush_mediamosa_check_mp4_h264($limit, $verbal);
    _drush_mediamosa_check_mime_type_octet_stream($limit, $verbal);

    _drush_mediamosa_check_duration($limit, $verbal);
    _drush_mediamosa_check_video_wo_video_codec($limit, $verbal);
    _drush_mediamosa_check_audio_wo_audio_codec($limit, $verbal);
    _drush_mediamosa_check_par($limit, $verbal);
  }
}

/*
 * Implementation of drush_COMMAND_validate().
 * Other way: drush_hook_COMMAND_validate().
 */
function drush_mediamosa_monitor_validate() {
  // Limit.
  $limit = drush_get_option('limit');
  if ($limit && !is_numeric($limit)) {
    return drush_set_error('LIMIT_IS_NOT_NUMERIC', dt('The limit parameter is not numeric. See "drush help mm-monitor".'));
  }

  // Verbal.
  $verbal = drush_get_option('verbal');
  if ($verbal && !in_array(drupal_strtolower($verbal), array('true', 'false'))) {
    return drush_set_error('VERBAL_IS_NOT_BOOLEAN', dt('The verbal parameter is not boolean. See "drush help mm-monitor".'));
  }
}

/**
 * Check mime type.
 * Collect the mediafiles, where there isn't mime type, but there is a filesize.
 */
function _drush_mediamosa_check_mime_type($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking mime-type]'));
  }

  // Prop ids.
  $prop_id_filesize = mediamosa_asset_mediafile_metadata_property::get_property_id_int('filesize');
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $filesize = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'filesize', 'mediafile.mediafile_id = filesize.mediafile_id and filesize.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_filesize);
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
  ));

  $query->fields('filesize', array(
    mediamosa_asset_mediafile_metadata_db::VAL_INT,
  ));

  // Condition.
  $query->isNotNull('filesize.' . mediamosa_asset_mediafile_metadata_db::VAL_INT);
  $query->condition(db_or()->isNull('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR)->condition('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, ''));
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles without mime type (and with filesize): @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Check magic mime files and h264 video codec.
 */
function _drush_mediamosa_check_mp4_h264($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking mime-type of h264]'));
  }

  // Prop ids.
  $prop_id_vcodec = mediamosa_asset_mediafile_metadata_property::get_property_id_int('video_codec');
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $filesize = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'vcodec', 'mediafile.mediafile_id = vcodec.mediafile_id and vcodec.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_vcodec);
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
  ));

  $query->fields('vcodec', array(
    mediamosa_asset_mediafile_metadata_db::VAL_INT,
  ));

  // Condition.
  $query->condition('vcodec.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, 'h264');
  $query->condition(db_or()->isNull('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR)->condition('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, ''));
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles without mime type (and with vcodec h264): @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Collect the mediafiles, where the mime type is application/octet-stream.
 */
function _drush_mediamosa_check_mime_type_octet_stream($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking mime-type octet stream]'));
  }

  // Prop ids.
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
    mediamosa_asset_mediafile_db::APP_ID,
  ));

  // Condition.
  $query->condition('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, 'application/octet-stream');
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles with mime type application/octet-stream: @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Check duration.
 */
function _drush_mediamosa_check_duration($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking duration]'));
  }

  // Prop ids.
  $prop_id_file_duration = mediamosa_asset_mediafile_metadata_property::get_property_id_char('file_duration');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $file_duration = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'file_duration', 'mediafile.mediafile_id = file_duration.mediafile_id and file_duration.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_file_duration);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  ));

  // Condition.
  $query->condition(db_or()->
    isNull('file_duration.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR)->
    condition('file_duration.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, '')->
    condition('file_duration.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, 0, '<')
  );
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles with duration problems: @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Check mime type = video without video codec.
 */
function _drush_mediamosa_check_video_wo_video_codec($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[mime type = video without video codec]'));
  }

  // Prop ids.
  $prop_id_video_codec = mediamosa_asset_mediafile_metadata_property::get_property_id_char('video_codec');
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');
  $prop_id_container_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('container_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $vide_codec = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'video_codec', 'mediafile.mediafile_id = video_codec.mediafile_id and video_codec.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_video_codec);
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);
  $container_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'container_type', 'mediafile.mediafile_id = container_type.mediafile_id and container_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_container_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  ));

  // Condition.
  //$query->isNotNull('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR);
  $query->where('SUBSTRING(mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .', 1, LOCATE(:location, mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .') - 1) = :type', array(
    ':location' => '/',
    ':type' => 'video',
  ));
  $query->condition(db_or()->isNull('video_codec.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR)->condition('video_codec.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, ''));
  $query->condition('container_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, 'asf', '<>');
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles with mime type = video and without video codec: @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Check mime type = audio without audio codec.
 */
function _drush_mediamosa_check_audio_wo_audio_codec($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[mime type = audio without audio codec]'));
  }

  // Prop ids.
  $prop_id_audio_codec = mediamosa_asset_mediafile_metadata_property::get_property_id_char('audio_codec');
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $audio_codec = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'audio_codec', 'mediafile.mediafile_id = audio_codec.mediafile_id and audio_codec.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_audio_codec);
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  ));

  // Condition.
  //$query->isNotNull('mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR);
  $query->where('SUBSTRING(mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .', 1, LOCATE(:location, mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .') - 1) = :type', array(
    ':location' => '/',
    ':type' => 'audio',
  ));
  $query->condition(db_or()->isNull('audio_codec.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR)->condition('audio_codec.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, ''));
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles with mime type = audio and without audio codec: @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}

/**
 * Check pixel and display aspect ratio.
 */
function _drush_mediamosa_check_par($limit, $verbal) {
  // Output header.
  if ($verbal) {
    drush_print(dt('[Checking pixel aspect ratio]'));
  }

  // Prop ids.
  $prop_id_width = mediamosa_asset_mediafile_metadata_property::get_property_id_int('width');
  $prop_id_height = mediamosa_asset_mediafile_metadata_property::get_property_id_int('height');
  $prop_id_mime_type = mediamosa_asset_mediafile_metadata_property::get_property_id_char('mime_type');

  // Build the query.
  $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile');

  // Joins.
  $width = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'width', 'mediafile.mediafile_id = width.mediafile_id and width.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_width);
  $height = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'height', 'mediafile.mediafile_id = height.mediafile_id and height.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_height);
  $mime_type = $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mime_type', 'mediafile.mediafile_id = mime_type.mediafile_id and mime_type.' . mediamosa_asset_mediafile_metadata_db::PROP_ID . ' = ' . $prop_id_mime_type);

  // Fields.
  $query->fields('mediafile', array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::ASSET_ID,
  ));

  // Condition.
  $query->condition(db_or()->
    isNull('width.' . mediamosa_asset_mediafile_metadata_db::VAL_INT)->
    condition('width.' . mediamosa_asset_mediafile_metadata_db::VAL_INT, 0, '<=')->
    isNull('height.' . mediamosa_asset_mediafile_metadata_db::VAL_INT)->
    condition('height.' . mediamosa_asset_mediafile_metadata_db::VAL_INT, 0, '<=')
  );
  $query->condition(db_or()->
    where('SUBSTRING(mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .', 1, LOCATE(:location, mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .') - 1) = :type', array(
      ':location' => '/',
      ':type' => 'video',
    ))->
    where('SUBSTRING(mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .', 1, LOCATE(:location, mime_type.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR .') - 1) = :type', array(
      ':location' => '/',
      ':type' => 'image',
    ))
  );
  $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

  // Count.
  $num_rows = $query->countQuery()->execute()->fetchField();
  if ($verbal) {
    drush_print(dt('Total number of mediafiles without width and/or height: @count', array('@count' => $num_rows)));
  }
  if ($num_rows > 0) {
    if ($verbal) {
      drush_print(dt('The first @limit links:', array('@limit' => $limit,)));
    }
    // Range.
    $query->range(0, $limit);
    $result = $query->execute();
    foreach ($result as $record) {
      if ($verbal) {
        drush_print(dt('mediafile_id: @id', array('@id' => $record[mediamosa_asset_mediafile_db::ID])) . ' ' . url('admin/mediamosa/browse/asset/' . $record[mediamosa_asset_mediafile_db::ASSET_ID], array('absolute' => TRUE)));
      }
      else {
        drush_print($record[mediamosa_asset_mediafile_db::ID]);
      }
    }
  }
}
