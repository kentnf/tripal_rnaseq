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
    'Expression Viewer' => t('/rnaseq/jbrowse'),
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
function tripal_rnaseq_get_reference_options($general_org) {

  $results = chado_select_record('organism', array('organism_id','common_name', 'genus', 'species'), array());
  foreach ($results as $result) {
    // skip the general orgamism name
    if ($result->organism_id > 9) {
       continue;
    }
    // get organism options base on general organism short name
    elseif ($result->genus == 'Citrullus' and $general_org == 'wm') {
      $organism_opts[$result->organism_id] = $result->common_name;
    }
    elseif ($result->species == 'sativus' and $general_org == 'cu') {
      $organism_opts[$result->organism_id] = $result->common_name;  
    }
    elseif ($result->species == 'melo' and $general_org == 'me') {
      $organism_opts[$result->organism_id] = $result->common_name;  
    }
    elseif ($result->genus == 'Cucurbita' and $general_org == 'pu') {
      $organism_opts[$result->organism_id] = $result->common_name;  
    }
    else {
      continue;
    }
  }
  return $organism_opts;
}

