<?php
/**
 * @file
 * Implements the RNASeq content type of node. 
 */

/**
 * Implements hook_node_info().
 *
 *  Provides information to drupal about the node types that we're creating
 *  in this module. 
 *
 * @ingroup tripal_rnaseq
 */
function tripal_rnaseq_node_info() {
  $nodes = array();
  $nodes['chado_rnaseq'] = array(
    'name' => t('Gene Expression'),
    'base' => 'chado_rnaseq',
    'description' => t('Load gene expression generated from RNASeq.'),
    'has_title' => TRUE,
    'locked' => TRUE,
    'chado_node_api' => array(
      'base_table' => 'project',
      'hook_prefix' => 'chado_rnaseq',
      'record_type_title' => array(
        'singular' => t('RNASeq'),
        'plural' => t('RNASeq'),
      ),
    )
  );
  return $nodes;
}

/**
 * Implements hook_node_access().
 * 
 * @param $node
 * Tee node on which the operation is performed, or, if it does not yet exist, the 
 * type of node to be created.
 *
 * @param $op
 * The operation to be performed. 
 * 
 * @param $account
 * A user object representing the user for whom the operation is to be performed. 
 *
 * @return 
 * If the permission for the specified operation is not set then return FALSE. If the
 * permissions is set then return NULL as this allwos other modules to disable
 * access. The only exception is when the $op == 'create'. We will always
 * return TRUE if the permission is set. 
 * 
 * @ingroup tripal_rnaseq
 * 
 */
function chado_rnaseq_node_access($node, $op, $account) {
  $node_type = $node;
  if (is_object($node)) {
    $node_type = $node->type;
  }

  if ($node_type == 'chado_rnaseq') {
    if ($op == 'create') {
      if (!user_access('create chado_rnaseq content', $account)) {
        return NODE_ACCESS_DENY;
      }
      return NODE_ACCESS_ALLOW;
    }
    if ($op == 'update') {
      if (!user_access('edit chado_rnaseq content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'delete') {
      if (!user_access('delete chado_rnaseq content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    if ($op == 'view') {
      if (!user_access('access chado_rnaseq content', $account)) {
        return NODE_ACCESS_DENY;
      }
    }
    return NODE_ACCESS_IGNORE;
  }
}

/**
 * Implements hook_form()
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_form($node, $form_state) {
  $form = array();

  // Not sure what this is for. Is this line necessary? 
  $form['#attribute']['enctype'] = 'multipart/form-data';
 
  // === get default value for the form ===
  if (property_exists($node, 'rnaseq')) {   
    $rnaseq = $node->rnaseq;
    $rnaseq_name = property_exists($node, 'rnaseq_name') ? $node->rnaseq_name : $rnaseq->name;
    $rnaseq_path = property_exists($node, 'rnaseq_path') ? $node->rnaseq_path : $rnaseq->path;
    $rnaseq_term = property_exists($node, 'rnaseq_term') ? $node->rnaseq_term : $rnaseq->type_id;
    $bioproject_id = property_exists($node, 'bioproject_id') ? $node->bioproject_id : $rnaseq->project_id;
    $organism_id = property_exists($node, 'organism_id') ? $node->organism_id : $rnaseq->organism_id;
  }
  else {
    $rnaseq_name = property_exists($node, 'rnaseq_name') ? $node->rnaseq_name : '';
    $rnaseq_path = property_exists($node, 'rnaseq_path') ? $node->rnaseq_path : '';
    $rnaseq_term = property_exists($node, 'rnaseq_term') ? $node->rnaseq_term : '';
    $bioproject_id = property_exists($node, 'bioproject_id') ? $node->bioproject_id : '';
    $organism_id = property_exists($node, 'organism_id') ? $node->organism_id : '';
  }

  // Obtain the list of BioProjects. 
  $sql = 'SELECT P.project_id, P.name FROM chado_bioproject B 
   LEFT JOIN {project} P ON P.project_id = B.project_id ORDER BY P.name';
  $results = chado_query($sql);
  $projects = array();
  while ($project = $results->fetchObject()) {
    $projects[$project->project_id] = $project->name;
  }

  $lt_message = tripal_set_message(t('If the bioproject is not present in the dropdown list, ')
    . l(t('add a new SRA Bioproject.'), 'node/add/chado-bioproject'),
    TRIPAL_INFO, array('return_html' => TRUE)
  );

  $form['bioproject_id'] = array(
    '#title' => t('BioProject'),
    '#type' => 'select',
    '#empty_value' => '',
    '#description' => t('Please select the BioProejct of the Expression file.'),
    '#default_value' => $bioproject_id,
    '#required' => TRUE,
    '#options' => $projects,
    '#suffix' => $lt_message,
  );

 
  $sql = 'SELECT organism_id, genus, species, common_name FROM {organism} ORDER BY genus';
  $results = chado_query($sql);
  $organisms = array();
  while ($organism = $results->fetchObject()) {
    $organisms[$organism->organism_id] = $organism->common_name;
  }

  $form['organism_id'] = array(
    '#title' => t('Organism'),
    '#type' => 'select',
    '#description' => t('Select the organism, the feature must match the selected orgianism.'),
    '#default_value' => $organism_id,
    '#options' => $organisms,
    '#required' => TRUE,
  );

  // select expression term 
  $exp_terms = array();
  $values = array('cv_id' => array('name'=>'tripal_rnaseq'),);
  $results = chado_select_record('cvterm', array('cvterm_id', 'name'), $values, array('return_array'=>1)); 
  foreach ($results as $r) {
    $exp_terms[$r->cvterm_id] = $r->name;
  } 

  $form['rnaseq_term'] = array(
    '#title' => t('Expression Term'),
    '#type' => 'select',
    '#empty_value' => '',
    '#description' => t('Please select the term for the Expression file.'),
    '#default_value' => $rnaseq_term,
    '#required' => TRUE,
    '#options' => $exp_terms,
  ); 

  // expression file path 
  $form['rnaseq_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of the RNASeq Expression File'),
    '#required' => TRUE,
    '#default_value' =>  $rnaseq_name,
  );
  $form['rnaseq_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path of the RNASeq Expression File'),
    '#required' => TRUE,
    '#default_value' => $rnaseq_path,
  );


  $form['rnaseq_load'] = array(
    '#type' => 'checkbox',
    '#title' => t('Load/Update Expression File for chado database'),
    '#default_value' => '',
  );

  return $form;
}

/**
 * Implements hook_validate().
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_validate($node, $form, &$form_state) {
  // Don't validate if a delete.
  if ($node->op == 'Delete') {
    return;
  }

  // We are syncing if there is an rnaseq node but no node ID. In this case a 
  // validation is not required.  
  if (is_null($node->nid) and property_exists($node, 'rnaseq') and $node->rnaseq != 0) {
    return;
  } 

  // Remove white space around field values.
  $node->rnaseq_name = trim($node->rnaseq_name);
  $node->rnaseq_path = trim($node->rnaseq_path);

  // Validate for an update. 
  // check if the changed bioproject_id and rnaseq term will affect other records
  // if not, pass for update
  $args = array(
    ':type_id'=> $node->rnaseq_term, 
    ':project_id'=>$node->bioproject_id, 
    ':organism_id'=>$node->organism_id
  );
  $sql = "SELECT nid FROM chado_rnaseq
    WHERE type_id=:type_id AND project_id=:project_id AND organism_id=:organism_id";

  if (property_exists($node, 'rnaseq_name')) {
    $status = db_query($sql, $args)->fetchField();
    // ***** code here has problem *****
    // update validate status => 2633523
    dpm($status, 'update validate status');
    if ($rr) {
      form_set_error('bioproject_name', t("Update cannot proceed. The bioproject
        '$node->bioproject_name' is already present in the database."));
      tripal_report_error('tripal_sra', TRIPAL_WARNING,
        'Update bioproject: name already exists: %value', 
        array('%value' => "name = $node->bioproject_name"));
    }
  }
  // Validate for an insert.
  // the bioproject_id rnaseq_term, and organism_id should unique
  // the name, path can be any content
  else {
    $args = array(
      ':type_id'=> $node->rnaseq_term, 
      ':project_id'=>$node->bioproject_id, 
      ':organism_id'=>$node->organism_id
    );
    $sql = "SELECT nid FROM chado_rnaseq
      WHERE type_id=:type_id AND project_id=:project_id AND organism_id=:organism_id";
    $status = db_query($sql, $args)->fetchField();

    if ($status) {
      form_set_error('rnaseq_term', 'BioProject, Expression Term and Organism should be unique.');
      form_set_error('bioproject_id', 'BioProject, Expression Term and Organism should be unique.');
      form_set_error('organism_id', 'BioProject, Expression Term and Organism should be unique.');
      tripal_report_error('tripal_rnaseq', TRIPAL_WARNING,
        'Insert RNASeq Expression: record already exists: %type_id %project_id %organism_id',
        array(
          '%type_id' => "Term = $node->rnaseq_term", 
          '%project_id' => "Project = $node->bioproject_id",
          '%project_id' => "Organism = $node->organism_id",
        )); 
    }
  }
}  

/**
 * Implements hook_insert().
 * 
 * This function is called on insert of a new node
 * of type 'chado_rnaseq' and inserts the necessary information to 
 * public chado_rnaseq table. 
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_insert($node) {
  
  // Remove white space around field values.
  $node->rnaseq_name = trim($node->rnaseq_name);
  $node->rnaseq_path = trim($node->rnaseq_path);

  // Actually insert the record.
  // get a tripal job to insert database
  $record = new stdClass();
  $record->nid         = $node->nid;
  $record->name        = $node->rnaseq_name;
  $record->path        = $node->rnaseq_path;
  $record->type_id     = $node->rnaseq_term;
  $record->project_id  = $node->bioproject_id;
  $record->organism_id = $node->organism_id;
  $status = drupal_write_record('chado_rnaseq', $record);
 
  // add expression value to chado.feature_expression using tripal_job
  if ($node->rnaseq_load == 1) {
    tripal_rnaseq_exp_load_job($node);
  }
} 

/**
 * Implements hook_load().
 * 
 *  When a node is requested by the user this function is called to allo us 
 *  to add auxiliary data to the node object.  
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_load($nodes) {
  foreach ($nodes as $nid => $node) {
    switch ($node->type) {
      case 'chado_rnaseq':
        $r = db_query("SELECT * FROM chado_rnaseq WHERE nid=:nid", array(':nid'=>$nid))->fetchObject();
        $node->rnaseq = $r;
        $node->title = $r->name;
    }
  } 
} 

/**
 * Implements hook_node_insert(). 
 * it only update node title and url 
 * 
 * @ingroup tripal_rnaseq
 */ 
function tripal_rnaseq_node_insert($node) {
  switch ($node->type) {
    case 'chado_rnaseq':
      dpm($node, 'tripal_rnaseq_node_insert');
      if ($node->rnaseq) {
        $node->title = $node->rnaseq->name;
      } else {
        $node->title = $node->rnaseq_name;
      } 
      break;
  }
}

/**
 * Implements hook_node_update().
 * This node update is different with content type update.
 * it only update node title
 *
 * @ingroup tripal_rnaseq
 */
function tripal_rnaseq_node_update($node) {
  switch ($node->type) {
    case 'chado_rnaseq':
      $node->title = $node->rnaseq->name;      
      break; 
  }
} 

/**
 * Implements hook_update().
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_update($node) {

  // Remove white space around field values.
  $node->rnaseq_name = trim($node->rnaseq_name);
  $node->rnaseq_path = trim($node->rnaseq_path);

  $values = array(
    'name'        => $node->rnaseq_name,
    'path'        => $node->rnaseq_path,
    'type_id'     => $node->rnaseq_term,
    'project_id'  => $node->bioproject_id,
    'organism_id' => $node->organism_id,
  );

  $status = db_update('chado_rnaseq')->fields($values)->condition('nid', $node->nid)->execute();

  if ($node->rnaseq_load == 1) {
    tripal_rnaseq_exp_load_job($node);
  }

  if (!$status) {
    drupal_set_message(t('Unable to update RNASeq Expression File.'), 'warning');
    tripal_report_error('tripal_rnaseq', TRIPAL_WARNING, 'Unable to update RNASeq Expression File where values: %values', array('%values' => print_r($values, TRUE)));
  } 
} 

/**
 * Implements hook_delete().
 * 
 * Delete bioproject from both drupal and chado databases. Check dependencies before
 * deleting from chado. 
 * 
 * @ingroup tripal_rnaseq
 */
function chado_rnaseq_delete($node) {

  // Remove data from drupal tables including chado_rnaseq, node, and node_revisions
  $sql_del = "DELETE FROM chado_rnaseq WHERE nid = :nid";
  db_query($sql_del, array(':nid' => $node->nid));
  $sql_del = "DELETE FROM node WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));
  $sql_del = "DELETE FROM node_revision WHERE nid = :nid AND vid = :vid";
  db_query($sql_del, array(':nid' => $node->nid, ':vid' => $node->vid));

  $message = "Warning: the RNASeq Expression File ". $node->title ." has been removed from chado_rnaseq table.
  But the Expression value still in chado.feature_expression which needs manually removed.
  ";
  drupal_set_message($message, 'status');
} 

/**
 * Implements hook_node_view().
 *
 * @ingroup tripal_rnaseq
 */

function tripal_rnaseq_node_view($node, $view_mode, $langcode) {
  switch ($node->type) {
      case 'chado_feature':
      if ($view_mode == 'full') {
        $node->content['tripal_rnaseq'] = array(
          '#markup' => theme('tripal_feature_rnaseq_expression', array('node' => $node)),
          '#tripal_toc_id' => 'rnaseq_expression',
          '#tripal_toc_title' => 'RNASeq Expression',
          '#weight' => -99,
        );
      }
      break;
  }
}


