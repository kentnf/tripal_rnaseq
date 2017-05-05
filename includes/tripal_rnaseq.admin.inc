<?php
/**
 * @file
 * 
 * This file contains the functions used for the administration
 * of the rnaseq content type.
 * 
 * @ingroup tripal_rnaseq
 */

/**
 * Administrative page callbacks for the rnaeq content type.
 * 
 * @return
 *   A form API array describing an administrative form.
 *
 * @ingroup tripal_rnaseq
 */
function tripal_rnaseq_admin() {

  $form = array();
 
  // rnaseq is not a chado node, do not have corresponding table in chado database, 
  // do not set title and url for rnaseq node. And the rnaseq node is only for load
  // expression data, user do not care about the processing of loading files. 

  $form['test'] = array(
    '#title' => t('Test'),
    '#description' => t('test for later'),
    '#type' => 'textarea',
  );

 return system_settings_form($form);
}

/**
 * Landing page for administration. Ensures Views are enabled and if not, provides links to do so.
 *
 * @return
 *   An HTML string that provides a link to enable the view.
 * 
 * @ingroup tripal_rnaseq
 */ 
function tripal_rnaseq_admin_rnaseq_view() {
  $output = '';
  
  // Set the breadcrumbs. 
  $breadcrumb = array(); //*** update this for tripal
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal SRA', 'admin/tripal/extension/tripal_sra');
  $breadcrumb[] = l('BioProject', 'admin/tripal/extension/tripal_sra/bioproject');
  drupal_set_breadcrumb($breadcrumb);

  // Add the view. 
  $view = views_embed_view('tripal_bioproject_admin_bioprojects', 'default');
  if (isset($view)) {
    $output .= $view;
  }
  else { 
    $output .= '<p>The Tripal Expression module uses primarily views to provide an '
      . 'administrative interface. Currently one or more views needed for this this ' 
      . 'administrative interface are disabled. <br><strong>Click each of the following links to ' 
      . 'enable the pertinent views</strong>:</b>'; //*** fix this wording
    $output .= '<ul>';
    $output .= '<li>' . l('BioProject View', 'admin/tripal/extension/tripal_sra/bioproject/views/bioprojects/enable') . '</li>';
    $output .= '</ul>';
  }
  return $output;
}

