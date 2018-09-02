<?php

/**
 * 
 */
function tripal_rnaseq_jbrowse_form($form, $form_state) {

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

  $form['project_id'] = array(
    '#type' => 'hidden',
    '#value' => $project_id,
  );

  $form['reference'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select a reference genome'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    //'#description' => 'the refernece genome is only used in read mapping, not the speicies or cultivar of sample',
  );

  $form['reference']['organism_id'] = array(
    '#type' => 'select',
    '#title' => '',
    '#options' => $ref_opts,
    '#default_value' => isset($_SESSION['tripal_rnaseq_analysis']['organism_id']) ? $_SESSION['tripal_rnaseq_analysis']['organism_id'] : '',
    '#required' => TRUE,
    //'#description' => t('select reference genome'),
  );

  // pairwise or time series
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#suffix' => '<br><br>',
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
function tripal_rnaseq_jbrowse_form_validate($form, &$form_state) {

}

/**
 *  
 */
function tripal_rnaseq_jbrowse_form_submit($form, $form_state) {

  $org_id     = $form_state['values']['organism_id'];
  $_SESSION['tripal_rnaseq_analysis']['organism_id'] = $org_id;
  drupal_goto("rnaseq/jbrowse");
}

