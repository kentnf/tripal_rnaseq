<?php

/**
 * @file
 * API for tripal_rnaseq
 */

/**
 * Implements hook_block_info
 * @return blocks
 */
function tripal_rnaseq_block_info () {
  $blocks = array();
  $blocks['rnaseq_menu'] = array(
    'info' => t('RNASeq Menu Block'),
  );
 
  return $blocks;
}

/** 
 * Implements hook_block_view().
 */
function tripal_rnaseq_block_view($delta='') {
  $block = array();
  switch($delta) {
    case 'rnaseq_menu' :
      $block['content'] = tripal_rnaseq_submenu();
      break;
  }
  return $block;
}

/**
 * @return submenu of sRNA 
 */
function tripal_rnaseq_submenu() {

  $current_path = current_path();
  if ($current_path == 'rnaseq/selectorg') {
    $current_path = 'rnaseq/jbrowse';
  }

  // get project id and general_org from session
  if (isset($_SESSION['tripal_rnaseq_analysis']['project_id'])
      && isset($_SESSION['tripal_rnaseq_analysis']['general_org']) ) {
  } else {
    drupal_set_message(t('Can not find any bioproject, please select from RNA-seq Collection.'), 'warning');
    drupal_goto("rnaseq/home");
  }

  $project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $general_org = $_SESSION['tripal_rnaseq_analysis']['general_org'];

  $menu_basic = array(
    'Home' 				=> t('/rnaseq/home'),
	'Info'				=> t('/rnaseq/' . $general_org . "/" . $project_id),
    'Heatmap' 			=> t('/rnaseq/heatmap'),
    'DEGs' 				=> t('/rnaseq/deg'),
    'Expression Viewer' => t('/rnaseq/selectorg'),
  ); 

  $menu = array();
  foreach ($menu_basic as $name => $path) {
    if ($path == '/'.$current_path) {
      $menu[] = "<font class=\"bg-primary\">$name</font>";
    }
    else {
      $menu[] = "<a href=\"$path\">$name</a>";
    }
  }

  $menu_html = '<h4 class="text-center text-info"><nav class="nav">' . implode(" | ", $menu) . '</nav></h4><br>';
  return $menu_html;
}

/**
 * @return organism options  
 */
function tripal_rnaseq_get_reference_options($project_id) {

  // get all the organism related to this project
  $sql = "SELECT O.organism_id, O.genus, O.species, O.common_name
    FROM chado.experiment E
    LEFT JOIN chado.biomaterial S ON E.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.organism O ON S.taxon_id = O.organism_id
    WHERE E.project_id = :project_id
  ";
  $args = array(':project_id' => $project_id);
  $results = db_query($sql, $args)->fetchAll();
  $orgs = array_unique($results, SORT_REGULAR);
  //dpm($orgs);

  $results = chado_select_record('organism', array('organism_id','common_name', 'genus', 'species'), array());
  foreach ($results as $result) {
    // skip the general orgamism name
	if ($result->organism_id == 10 || $result->organism_id == 11 || $result->organism_id == 12) {
       continue;
    }
    // get organism options base on general organism short name
    elseif ($result->genus == 'Citrullus') {
      $wm_opts[$result->organism_id] = $result->common_name;
    }
    elseif ($result->species == 'sativus') {
      $cu_opts[$result->organism_id] = $result->common_name;  
    }
    elseif ($result->species == 'melo') {
      $me_opts[$result->organism_id] = $result->common_name;  
    }
    else {
      continue;
    }
  }
 
  $organism_opts = array();
  foreach ($orgs as $org) {
    if ($org->organism_id == 10) { //common watermelon
	  $organism_opts = $wm_opts + $organism_opts;
	}
	elseif ($org->organism_id == 11) { //common cucumber
	  $organism_opts = $cu_opts + $organism_opts;
	}
	elseif ($org->organism_id == 12) { //common melon
      $organism_opts = $me_opts + $organism_opts;
	}
	else {	// RNA-Seq reads align to specific organism
      $organism_opts[$org->organism_id] = $org->common_name;
	}
  }
  return $organism_opts;
}

