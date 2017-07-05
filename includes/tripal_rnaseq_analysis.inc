<?php

/**
 * 
 */
function tripal_rnaseq_analysis_form($form, $form_state) {

  // drupal_set_title(t('New title for the node form'));
  dpm($_SESSION['tripal_rnaseq_analysis']);
  // dpm($_SESSION['tripal_rnaseq_heatmap']['experiments']);

  $project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $general_org = $_SESSION['tripal_rnaseq_analysis']['general_org'];
  $ref_opts = tripal_rnaseq_get_reference_options($general_org); 

  $form['reference'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select Reference Genome'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => 'the refernece genome is only used in read mapping, not the speicies or cultivar of sample',
  );

  $form['reference']['organism_id'] = array(
    '#type' => 'select',
    '#title' => '',
    '#options' => $ref_opts,
    '#default_value' => isset($_SESSION['tripal_rnaseq_analysis']['organism_id']) ? $_SESSION['tripal_rnaseq_analysis']['organism_id'] : '',
    '#required' => TRUE,
    '#description' => t('select reference genome'),
  );

  $form['query'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enter the list of IDs.'),
    '#description' => t('description of input ids'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  // Checkbox to show an example.
  $form['query']['example_id'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show an Example ID'),
    '#prefix' => '<span style="float: right;">',
    '#suffix' => '</span>',
    '#ajax' => array(
      'callback' => 'ajax_rnaseq_analysis_example_id_callback',
      'wrapper'  => 'id-textarea',
      'method'   => 'replace',
      'effect'   => 'fade',
    ),
  );
  // Textfield for submitting a ID list for query
  $form['query']['ID'] = array(
    '#type' => 'textarea',
    '#title' => t('Enter gene IDs'),
    '#description'=>t('Enter gene IDs in the text area.'),
    '#default_value' => isset($_SESSION['tripal_rnaseq_analysis']['genes_str']) ? $_SESSION['tripal_rnaseq_analysis']['genes_str'] : '',
    '#prefix' => '<div id="id-textarea">',
    '#suffix' => '</div>',
  );
  // Upload a file as an alternative to enter a query ID
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['query']['UPLOAD'] = array(
    '#title' => 'Or upload the file containing the list of gene IDs:  ',
    '#type' => 'managed_file',
    '#description' => t('The file should be a plain-text
  (.txt) file. Max size: %max_size. <strong>Don\'t forget to press the Upload button before
  attempting to submit your ID.</strong>',
      array(
        '%max_size' => round(file_upload_max_size() / 1024 / 1024,1) . 'MB'
      )
    ),
    '#upload_validators' => array(
      //'file_validate_extensions' => array('txt'),
      'file_validate_size' => array(file_upload_max_size()),
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['#prefix'] = '<div class="row"> <div class="col-md-8 col-md-offset-2">';
  $form['#suffix'] = '</div> </div>';

  return $form;
}

/**
 * 
 */
function tripal_rnaseq_analysis_form_validate($form, &$form_state) {

  if (empty($form_state['values']['organism_id'])) {
    form_set_error('organism_id', t('No genome selected. Choose a reference genome from the list.'));
  }
  $organism_id = $form_state['values']['organism_id'];

  // Validate Query
  //----------------
  // @todo: We are currently not validating uploaded files are valid FASTA.
  // First check to see if we have an upload & if so then validate it.
  $file = file_load($form_state['values']['UPLOAD']);

  // If the $file is populated then this is a newly uploaded, temporary file.
  if (is_object($file)) {
    $form_state['qFlag'] = 'upQuery';
    $form_state['upQuery_path'] = drupal_realpath($file->uri);
  }
  // Otherwise there was no file uploaded.
  // Check if there was a query sequence entered in the texfield.
  elseif (!empty($form_state['input']['ID'])) {
    // Check to ensure that the query sequence entered is valid FASTA.
    // if (validate_fasta_sequence($type, $form_state['input']['ID'])){
    //  form_set_error('query', t('You need to provide a valid FASTA sequence '
    //  . 'for the query. It should contain a FASTA header/definition line '
    //  . 'followed by %molecule-type. For more information see the '
    //  . '<a href="@url" target="_blank">NCBI FASTA Specification</a>.',
    //    array(
    //      '%molecule-type' => $molecule_type,
    //      '@url' => 'http://www.ncbi.nlm.nih.gov/BLAST/blastcgihelp.shtml'
    //  )));
    //}
    //else {
      $form_state['qFlag'] = 'seqQuery';
    //}
  }
  // Otherwise they didn't enter a query!!
  else {
    form_set_error('query', t('No gene IDs given. Enter IDs in the box provided '
    . 'or upload a plain text file.'));
  }

  // Validate ID list
  if (!empty($form_state['values']['ID']) && $form_state['values']['ID'] == 'unknown dataset type') {
    form_set_error('query', t('No gene IDs given.'));
  }
}

/**
 *  
 */
function tripal_rnaseq_analysis_form_submit($form, $form_state) {

  // load gene list to array
  // ** will do it later ***
  /**
  if (isset($form_state['qFlag'])) {
    if ($form_state['qFlag'] == 'seqQuery') {
      $seq_content = $form_state['values']['ID'];
      $goenrichjob['gene_list'] = '/tmp/' . date('YMd_His') . '_GO_gene_list.txt';
      file_put_contents ($goenrichjob['gene_list'], $seq_content);
    }
    elseif ($form_state['qFlag'] == 'upQuery') {
      $goenrichjob['gene_list'] = $form_state['upQuery_path'];
    }
  }
  */

  $id_content = $form_state['values']['ID'];
  $_SESSION['tripal_rnaseq_analysis']['genes_str'] = $id_content;
  $_SESSION['tripal_rnaseq_analysis']['organism_id'] = $form_state['values']['organism_id'];

  //$ids = explode("\n", $id_content, -1);
  //$_SESSION['tripal_rnaseq_analysis']['genes'] = $ids;
  drupal_goto("rnaseq/analysisresult");
}

/**
 * display tripal_rnaseq_analysis_result 
 * 
 * @param NA
 *
 * @return heatmap
 * 
 */
function tripal_rnaseq_analysis_result() {

  // load data from session
  $project_id = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $organism_id = $_SESSION['tripal_rnaseq_analysis']['organism_id'];
  $genes_str = $_SESSION['tripal_rnaseq_analysis']['genes_str'];
  $gs = explode("\n", $genes_str, -1);
  $genes = Array();
  foreach ($gs as $gene) {
    $genes[] = rtrim($gene);
  }

  $enames = $_SESSION['tripal_rnaseq_heatmap']['experiments']; 

  // get expression value
  $sql = "SELECT EXP.value,
      C.name as type_name,
      P.project_id, P.name as project_name,
      S.biomaterial_id, S.name as biosample_name,
      E.experiment_id, E.name as sample_name,
      F.name as feature_name
    FROM chado.feature_expressionprop EXP
    LEFT JOIN chado.feature_expression EX ON EXP.feature_expression_id = EX.feature_expression_id
    LEFT JOIN chado.feature F on EX.feature_id = F.feature_id
    LEFT JOIN chado.cvterm C ON EXP.type_id = C.cvterm_id
    LEFT JOIN chado.experiment E ON EX.experiment_id = E.experiment_id
    LEFT JOIN chado.biomaterial S ON E.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.project P ON E.project_id = P.project_id
    WHERE
      P.project_id = :project_id AND 
      C.name = :type_name AND
      F.organism_id = :organism_id AND
      F.uniquename in (:genes) AND
      E.name in (:enames)
    ORDER BY EXP.value::real ASC
  ";

  $args = array(
    ':project_id' => $project_id,
    ':type_name'=> 'RPKM',
    ':organism_id' => $organism_id,
    ':enames'=>$enames,
    ':genes'=>$genes);
  $results = db_query($sql, $args)->fetchAll();
  $exp = array();
  foreach ($results as $r) {
    if (isset($exp[$r->feature_name])) {
      $exp[$r->feature_name][$r->sample_name] = $r->value;
    } else {
      $exp[$r->feature_name] = array();
      $exp[$r->feature_name][$r->sample_name] = $r->value;
    }
  }

  return theme('tripal_rnaseq_analysis', array('exp' => $exp));
}

/**
 * AJAX: Replace the ID textarea with one containing an example.
 */
function ajax_rnaseq_analysis_example_id_callback($form, $form_state) {
  $select_org = $form_state['values']['organism_id'];
  $example_ids = 'unknown';

  // get gene type_id 
  $values = array(
    'name' => 'gene',
    'is_obsolete' => 0,
    'cv_id' => array (
       'name' => 'sequence',
     ),
  );
  $result = chado_select_record('cvterm', array('cvterm_id', 'name'), $values);
  if (empty($result)) {
    drupal_set_message("tripal_rnaseq: can not find type_of of gene", 'error');
  }
  $type_id = $result[0]->cvterm_id;

  // query db to get example ID
  $sql = "SELECT F.feature_id, F.name FROM chado.feature F
    WHERE F.type_id = :type_id AND F.organism_id = :organism_id 
    LIMIT 20
  ";
  $args = array(':type_id' => $type_id, ':organism_id' => $select_org);
  $result = db_query($sql, $args);
 
  $example_ids = '';
  foreach ($result as $gene) {
    $example_ids = $example_ids . $gene->name . "\n"; 
  }

  // If the Show Example checkbox is true then put the example in the textfield
  if ($form_state['values']['example_id']) {
    $form['query']['ID']['#value'] = $example_ids;
  }
  // Otherwise we want to remove the already displayed example.
  else {
    $form['query']['ID']['#value'] = '';
  }

  return $form['query']['ID'];
}

