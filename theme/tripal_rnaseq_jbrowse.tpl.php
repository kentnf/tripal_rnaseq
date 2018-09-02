<?php 
  // [JBrowse] get RNA-seq tracks according to project and samples
  if ( isset($_SESSION['tripal_rnaseq_analysis']['project_id']) ) {
    if ( isset($_SESSION['tripal_rnaseq_analysis']['organism_id']) ) {
    } else {
      drupal_goto("rnaseq/selectorg");
    } 
  }
  else {
    drupal_set_message(t('Can not find any bioproject, please select from RNA-seq Collection.'), 'warning');
    drupal_goto("rnaseq/home");
  }

  $project_id = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $bioproject = chado_generate_var('project', array('project_id' => $project_id));
  $project_name = $bioproject->name;

  $jbrowse_tracks = "DNA,genes";

  //get all samples of selected project_id 
  foreach ($_SESSION['tripal_rnaseq_analysis']['samples'] as $sname) {
    $track_name = $project_name . "-" . $sname;
    $jbrowse_tracks.= ",$track_name";
  }

  // [JBrowse] determine the data according to orgamism ID
  $organism_id = $_SESSION['tripal_rnaseq_analysis']['organism_id'];
  $jbrowse_data = NULL;
  if ($organism_id == 1) {
    $jbrowse_data = 'watermelon_v1';
  } elseif ($organism_id == 2) {
    $jbrowse_data = 'cucumber_v2';
  } elseif ($organism_id == 3) {
    $jbrowse_data = 'melon_v351';
  } elseif ($organism_id == 4) {
    $jbrowse_data = 'wcg_v2';
  } elseif ($organism_id == 6) {
    $jbrowse_data = 'cucumber_PI';
  } elseif ($organism_id == 7) {
    $jbrowse_data = 'cucumber_gy14';
  } elseif ($organism_id == 8) {
    $jbrowse_data = 'cma';
  } elseif ($organism_id == 9) {
    $jbrowse_data = 'cmo';
  } elseif ($organism_id == 13) {
    $jbrowse_data = 'cpe';
  } elseif ($organism_id == 14) {
    $jbrowse_data = 'lsi';
  } else {
    $jbrowse_data = NULL;
  }

  # $jbrowse_link = "/JBrowse/?data=icugi_data/json/$jbrowse_data&tracks=$jbrowse_tracks&tracklist=1&nav=1&overview=0";

  //dpm($jbrowse_link);
  $options = array(
    'query' => array(
      'data'=>'icugi_data/json/'.$jbrowse_data,
      'tracks' => $jbrowse_tracks,
      'tracklist' => 1,
      'nav' => 1,
      'overview' => 0,
    ),
  );

  drupal_goto('/JBrowse', $options);

