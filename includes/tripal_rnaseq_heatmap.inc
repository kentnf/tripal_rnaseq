<?php

/**
 * 
 */
function tripal_rnaseq_heatmap_form($form, $form_state) {

  // drupal_set_title(t('New title for the node form'));
  // dpm($_SESSION['tripal_rnaseq_analysis']);

  if (isset($_SESSION['tripal_rnaseq_analysis']['project_id'])
      && isset($_SESSION['tripal_rnaseq_analysis']['general_org']) ) {

  } else {
    drupal_set_message(t('Can not find any bioproject, please select from RNA-seq Collection.'), 'warning');
    drupal_goto("rnaseq/home");
  }

  $project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $general_org = $_SESSION['tripal_rnaseq_analysis']['general_org'];
  $ref_opts = tripal_rnaseq_get_reference_options($project_id); 

  // get project info
  $bioproject = chado_generate_var('project', array('project_id' => $project_id));
  $bioproject = chado_expand_var($bioproject, 'field', 'project.description');
  $bioproject_url = l($bioproject->name, '/rnaseq/' . $general_org . '/' . $project_id);
  $project_info_html = "The project $bioproject_url has been selected.<br>";

  $form['reference'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select a reference genome'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    //'#description' => 'The refernece genome is only used in read mapping, not the speicies or cultivar of sample',
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
    '#title' => t('Enter the list of gene IDs.'),
    '#description' => t('Only gene IDs are accepted. The maximum input number is 1000.'),
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
    '#title' => t(''),
    '#description'=>t('Enter gene IDs in the text area.'),
    '#default_value' => isset($_SESSION['tripal_rnaseq_analysis']['genes_str']) ? $_SESSION['tripal_rnaseq_analysis']['genes_str'] : '',
    '#prefix' => '<div id="id-textarea">',
    '#suffix' => '</div>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['#prefix'] = "
    <div class=\"row\"><div class=\"col-md-8 col-md-offset-2\">
      <div class=\"alert alert-dismissable alert-info\">
        <strong>$project_info_html</strong>
      </div>";
  $form['#suffix'] = '</div> </div>';

  return $form;
}

/**
 * 
 */
function tripal_rnaseq_heatmap_form_validate($form, &$form_state) {

  if (empty($form_state['values']['organism_id'])) {
    form_set_error('organism_id', t('No genome selected. Choose a reference genome from the list.'));
  }
  $organism_id = $form_state['values']['organism_id'];

  // Validate ID list
  if (empty($form_state['values']['ID'])) {
    form_set_error('query', t('No gene IDs given.'));
  }
  else 
  {
    if ($form_state['values']['ID'] == 'unknown dataset type') {
      form_set_error('query', t('No gene IDs given.'));
    }
    else {
      $ids = explode("\n", $form_state['values']['ID'], -1);
      $genes = array();
      foreach ($ids as $id) {
        $genes[trim($id)] = 1;
      }

      if (sizeof($genes) < 5) {
        form_set_error('query', t("Only accept IDs no less than 5\n"));
      }

      if (sizeof($genes) > 1000) {
        form_set_error('query', t("Only accept IDs no more than 1000\n"));
      }

      $sql = "SELECT uniquename FROM chado.feature 
         WHERE
           organism_id = :organism_id AND 
           uniquename in (:genes)
      ";

      $args = array(
        ':organism_id' => $organism_id,
        ':genes'=> array_keys($genes)
      );
      $results = db_query($sql, $args)->fetchAll();
      foreach ($results as $r) {
        unset($genes[$r->uniquename]);
      }

      if (sizeof($genes) > 0) {
        $error_genes = implode("<br>\n", array_keys($genes)); 
        form_set_error('query', t("Can not find below IDs, please check them:<br>\n" . $error_genes));
      }
    }
  }
}

/**
 *  
 */
function tripal_rnaseq_heatmap_form_submit($form, $form_state) {

  // load gene list to array
  $id_content = $form_state['values']['ID'];
  $_SESSION['tripal_rnaseq_analysis']['genes_str'] = $id_content;
  $_SESSION['tripal_rnaseq_analysis']['organism_id'] = $form_state['values']['organism_id'];

  //$ids = explode("\n", $id_content, -1);
  //$_SESSION['tripal_rnaseq_analysis']['genes'] = $ids;
  drupal_goto("rnaseq/heatmapresult");
}

/**
 * display tripal_rnaseq_analysis_result 
 * 
 * @param NA
 *
 * @return heatmap
 * 
 */
function tripal_rnaseq_heatmap_result() {

  // load data from session
  $project_id = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $organism_id = $_SESSION['tripal_rnaseq_analysis']['organism_id'];
  $genes_str = $_SESSION['tripal_rnaseq_analysis']['genes_str'];
  $gs = explode("\n", $genes_str, -1);
  $genes = Array();
  foreach ($gs as $gene) {
    $genes[] = rtrim($gene);
  }

  $enames = $_SESSION['tripal_rnaseq_analysis']['samples']; 

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
  $exp_min_value = 100;
  foreach ($results as $r) {
    if ($r->value > 0 && $r->value < $exp_min_value) {
      $exp_min_value = $r->value;
    }
    if (isset($exp[$r->feature_name])) {
      $exp[$r->feature_name][$r->sample_name] = $r->value;
    } else {
      $exp[$r->feature_name] = array();
      $exp[$r->feature_name][$r->sample_name] = $r->value;
    }
  }
  
  // replace non-express value to min-express value
  foreach ($exp as $feature_name => $exp_sample) {
    foreach ($exp_sample as $sample_name => $exp_value) {
      if ($exp[$feature_name][$sample_name] == 0) {
        $exp[$feature_name][$sample_name] = $exp_min_value;
      }
    }
  }

  return theme('tripal_rnaseq_heatmap', array('exp' => $exp, 'project_id'=>$project_id));
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

