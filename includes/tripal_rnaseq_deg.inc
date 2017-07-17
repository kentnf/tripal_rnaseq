<?php

/**
 * 
 */
function tripal_rnaseq_deg_form($form, $form_state) {

  // drupal_set_title(t('New title for the node form'));
  dpm($_SESSION['tripal_rnaseq_analysis']);
  dpm($_SESSION['tripal_rnaseq_heatmap']);

  $project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
  $general_org = $_SESSION['tripal_rnaseq_analysis']['general_org'];
  $ref_opts = tripal_rnaseq_get_reference_options($general_org); 

  $form['project_id'] = array(
    '#type' => 'hidden',
    '#value' => $project_id,
  );

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

  // select two samples from experiments (biosample)
  $sample_opts = array();
  foreach ($_SESSION['tripal_rnaseq_heatmap']['experiments'] as $sname) {
    $sample_opts[$sname] = $sname;
  }

  $form['pairwise'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select two samples for pairwise comparison.'),
    '#description' => t('description of input ids'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['pairwise']['sample_a'] = array(
    '#type' => 'select',
    '#title' => t('Sample A'),
    '#options' => $sample_opts,
    '#prefix' => '<span style="float: left; padding: 10px;">',
    '#suffix' => '</span>',
  );

  $form['pairwise']['sample_b'] = array(
    '#type' => 'select',
    '#title' => t('VS. Sample B'),
    '#options' => $sample_opts,
    '#prefix' => '<span style="float: left; padding: 10px;">',
    '#suffix' => '</span>',
  );

  // advance option
  $form['advance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Options.'),
    //'#description' => t('description of input ids'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $program_opts = array('DESeq' => 'DESeq', 'edgeR' => 'edgeR');
  $form['advance']['program'] = array(
    '#type' => 'select',
    '#title' => t('Program'),
    '#options' => $program_opts,
    '#default_value' => 'edgeR',
  );

  $form['advance']['ratio'] = array(
    '#type' => 'textfield',
    '#title' => t('Ratio'),
    '#maxlength' => 20,
    '#default_value' => '2',
  );

  $form['advance']['adjp'] = array(
    '#type' => 'textfield',
    '#title' => t('Adjusted P-value'),
    '#maxlength' => 20, 
    '#default_value' => '0.05',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#suffix' => '<br><br>',
  );

  /**
  $form['timeseries'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select three or more samples for time series comparison.'),
    '#description' => t('description of input ids'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $n = 0;
  foreach ($sample_opts as $sample_id => $sample_name) {
    $label = "sample_" . $sample_id;
    $n++;
    if ($n == 1) {
      $form['timeseries'][$label] = array(
        '#type' => 'checkbox',
        '#title' => $sample_name,
        //'#default_value' => isset($_SESSION['tripal_synview_search']['REFORG'][$org_id]) ? 1 : 0,
        '#prefix' => '<br><b>Please choose three samples at least:</b><br><span style="float: left; display: block; margin: 10px;">',
        '#suffix' => '</span>',
      );
    } else {
      $form['timeseries'][$label] = array(
        '#type' => 'checkbox',
        '#title' => $sample_name,
        //'#default_value' => isset($_SESSION['tripal_synview_search']['REFORG'][$org_id]) ? 1 : 0,
        '#prefix' => '<span style="float: left; display: block; margin: 10px;">',
        '#suffix' => '</span>',
      );
    }
  }

  $form['submit_b'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
  );
  */

  $form['#prefix'] = '<div class="row"> <div class="col-md-8 col-md-offset-2">';
  $form['#suffix'] = '</div> </div>';

  return $form;
}

/**
 * 
 */
function tripal_rnaseq_deg_form_validate($form, &$form_state) {

  // check user input
  $org_id     = $form_state['values']['organism_id'];
  $sample_a   = $form_state['values']['sample_a'];
  $sample_b   = $form_state['values']['sample_b'];
  $ratio      = $form_state['values']['ratio'];
  $adjp       = $form_state['values']['adjp'];
  $project_id = $form_state['values']['project_id'];

  if ($sample_a == $sample_b) {
    form_set_error('pairwise', t('Please select different samples for pairwise comparison.'));
  }

  if (!empty($ratio) && is_numeric($ratio) && $ratio > 1) {
  } else {
    form_set_error('ratio', t('Please enter a positive value (>1) for the ratio cutoff.'));
  }
 
  if (!empty($adjp) && is_numeric($adjp) && $adjp < 1 && $adjp > 0) {
  } else {
    form_set_error('adjp', t('Please enter a positive value (0~1) for the adjust p-value cutoff.'));
  }
}

/**
 *  
 */
function tripal_rnaseq_deg_form_submit($form, $form_state) {

  // check user input
  $org_id     = $form_state['values']['organism_id'];
  $sample_a   = $form_state['values']['sample_a'];
  $sample_b   = $form_state['values']['sample_b'];
  $ratio      = $form_state['values']['ratio'];
  $adjp       = $form_state['values']['adjp'];
  $project_id = $form_state['values']['project_id'];
  $rscript    = $form_state['values']['program'];

  // check if the data has biological duplicate
  // then check rpkm and raw count is exist 
  $error = 0;

  $sql = 'SELECT R.*, C.name as type_name FROM chado_rnaseq R
    LEFT JOIN chado.cvterm C ON R.type_id = C.cvterm_id
    WHERE project_id = :project_id AND organism_id = :organism_id';
  $arg = array(':project_id'=>$project_id, ':organism_id' => $org_id);
  $results = db_query($sql, $arg)->fetchAll();

  $raw_count_file = null;
  $rpkm_file = null;
  if (sizeof($results) >= 2) {
    foreach ($results as $result) {
      if (isset($result->type_name) && isset($result->path) && preg_match("/^SD/", $result->type_name)) {
        $path = $result->path;
        $path = preg_replace("/_sd.txt$/i", "", $path);
        $raw_count_file = $path . "_raw.txt";
        $rpkm_file = $path . ".txt";
      }  
    }
  } else {
    $error = 1;
  }

  if (!is_readable($raw_count_file) || !is_readable($rpkm_file)) {
    $error = 1;
  }

  // submit job for DEG
  //$error = 1; 
  if (!$error) {
    global $user;
    $output_dir = variable_get('file_public_path', conf_path() . DIRECTORY_SEPARATOR .  'files') . 
      DIRECTORY_SEPARATOR . 'tripal' . DIRECTORY_SEPARATOR . 'tripal_rnaseq';
    $output_file = $output_dir . DIRECTORY_SEPARATOR . date('YMd_His') . '.deg.txt';
    $deg_program = DRUPAL_ROOT . DIRECTORY_SEPARATOR . drupal_get_path('module', 'tripal_rnaseq'). DIRECTORY_SEPARATOR . 'DEGtool.pl';

    $job_args = array(
      'program' => $deg_program,
      'rscript' => $rscript,
      'raw'     => $raw_count_file,
      'rpkm'    => $rpkm_file,
      'sampleA' => $sample_a,
      'sampleB' => $sample_b, 
      'ratio'   => $ratio,
      'adjp'    => $adjp,
      'output'  => $output_file,
    );

    $job_id = tripal_add_job(
      t('DEG identification between @sampleA and @sampleB for @project_name', array(
        '@sampleA'=>$sample_a, '@sampleB'=>$sample_b, '@project_name' =>$project_name)),
      'tripal_rnaseq',
      'tripal_rnaseq_deg_job_run',
      $job_args,
      $user->uid
    );

    //Encode the job_id
    // $job_encode_id = tripal_rnasq_make_secret($job_id);
    drupal_goto("rnaseq/degresult/$job_id");
  }

  //$_SESSION['tripal_rnaseq_analysis']['genes'] = $ids;
}

/**
 * tripal_rnaseq_deg_job_run 
 */
function tripal_rnaseq_deg_job_run($program, $rscript, $raw, $rpkm, $sampleA, $sampleB, $ratio, $adjp, $output, $job_id = NULL) {

  $deg_cmd = escapeshellarg($program);
  $deg_cmd.= ' -s ' . escapeshellarg($rscript);
  $deg_cmd.= ' -i ' . escapeshellarg($raw);
  $deg_cmd.= ' -r ' . escapeshellarg($rpkm);
  $deg_cmd.= ' -a ' . escapeshellarg($sampleA);
  $deg_cmd.= ' -b ' . escapeshellarg($sampleB);
  $deg_cmd.= ' -f ' . escapeshellarg($ratio);
  $deg_cmd.= ' -p ' . escapeshellarg($adjp);
  $deg_cmd.= ' -o ' . escapeshellarg($output);

  print "\nExecuting the command for DEG:\n" . $deg_cmd . "\n";
  system($deg_cmd);

  if (!file_exists($output)) {
    tripal_report_error(
      'tripal_rnaseq',
      'TRIPAL_ERROR',
      "Error in DEG identification: %cmd.",
      array('%cmd'=>$deg_cmd),
      array('print'=>TRUE)
    );
    return FALSE;
  }

  print "done"; 
}


/**
 * display tripal_rnaseq_analysis_result 
 * 
 * @param NA
 *
 * @return heatmap
 * 
 */
function tripal_rnaseq_deg_result() {

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

  return theme('tripal_rnaseq_deg_result', array('exp' => $exp, 'project_id'=>$project_id));
}

/**
 * AJAX: Replace the ID textarea with one containing an example.
 */
function ajax_rnaseq_deg_example_id_callback($form, $form_state) {
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

