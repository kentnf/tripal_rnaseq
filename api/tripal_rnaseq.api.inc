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

  $menu = array();
  $menu[] = l('Home', 'rnaseq/home');
  //$menu[] = l('Search', 'rnaseq/search');
  $menu[] = l('Anaysis', 'rnaseq/analysis');
  $menu[] = l('DEGs', 'rnaseq/deg');
  $menu[] = l('Expression Viewer', 'rnaseq/view');
  $menu_html = '<h4 class="text-center text-info">' . implode(" | ", $menu) . '</h4><br>';
  return $menu_html;
}

/**
 * @return organism options  
 */
function tripal_rnaseq_get_reference_options($general_org) {

  $results = chado_select_record('organism', array('organism_id','common_name', 'genus', 'species'), array());
  foreach ($results as $result) {
    if ($result->genus == 'Citrullus' and $general_org == 'wm') {
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






