<?php

// retrun to home page if there is no project selected
if (isset($_SESSION['tripal_rnaseq_analysis']['project_id'])
  && isset($_SESSION['tripal_rnaseq_analysis']['general_org']) ) {
} else {
  drupal_set_message(t('Can not find any bioproject, please select from RNA-seq Collection.'), 'warning');
  drupal_goto("rnaseq/home");
}
$project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
$general_org = $_SESSION['tripal_rnaseq_analysis']['general_org'];

// get bioproject and biosample information
$bioproject = chado_generate_var('project', array('project_id' => $project_id));
$biosample = chado_generate_var('biomaterial', array('biomaterial_id' => $sample_id));
$biosample = chado_expand_var($biosample, 'field', 'biomaterial.description');

// dispaly breadcrumb
$nav = array();
$nav[] = l('Home', 'rnaseq/home');
$nav[] = l($bioproject->name . ' (Bioproject)', 'rnaseq/' . $general_org . '/' . $project_id);
$nav[] = t($biosample->name . ' (BioSample)');

$breadcrumb = '<nav class="nav">' . implode(" > ", $nav) . '</nav><br>';
print $breadcrumb;

?>
  <div class="tripal_biomaterial-data-block-desc tripal-data-block-desc"><h4>Overview of this BioSample:</h4></div>
  <?php

  $headers = array();
  $rows = array();

  // The biosample name.
  $rows[] = array(
    array(
      'data' => 'Biosample',
      'header' => TRUE,
      'width' => '20%',
    ),
    $biosample->name
  );

  // The organism from which the biosample was collected
  if($biosample->taxon_id) {
    $organism =  '<i>' . $biosample->taxon_id->genus . ' ' . $biosample->taxon_id->species . '</i>';
    if (property_exists($biosample->taxon_id, 'nid')) {
      $organism =  l('<i>' . $biosample->taxon_id->genus . ' ' . $biosample->taxon_id->species . '</i>', 'node/' . $biosample->taxon_id->nid, array('html' => TRUE));
    }
    $rows[] = array(
      array(
        'data' => 'Organism',
        'header' => TRUE,
        'width' => '20%',
      ),
    $organism
    );
  }

  // The biosource provider
  if($biosample->biosourceprovider_id) {
    $rows[] = array(
      array(
        'data' => 'Biosample Provider',
        'header' => TRUE,
        'width' => '20%',
      ),
      '<i>' . $biosample->biosourceprovider_id->name . '</i>'
    );
  }

  // allow site admins to see the biomaterial ID
  if (user_access('view ids')) {
    // Biomaterial ID
    $rows[] = array(
      array(
        'data' => 'BioSample ID',
        'header' => TRUE,
        'class' => 'tripal-site-admin-only-table-row',
      ),
      array(
        'data' => $biosample->biomaterial_id,
        'class' => 'tripal-site-admin-only-table-row',
      ),
    );
  }

  // Generate the table of data provided above. 
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_biomaterial-table-base',
      'class' => 'tripal-biomaterial-data-table tripal-data-table',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // Print the table and the description.
  print theme_table($table);

  // Print the biomaterial description.
  if ($biosample->description) { ?>
    <div style="text-align: justify"><?php print $biosample->description?></div> <?php
  }

  /**
   * display properties
   */
  $biosample = chado_expand_var($biosample, 'table', 'biomaterialprop', array('return_array' => 1));
  $properties = $biosample->biomaterialprop;

  if (count($properties) > 0) {

    ?>
    <br>
    <div class="tripal_bioproject-data-block-desc tripal-data-block-desc"><h4>Properties this BioSample:</h4></div>
    <?php

    $headers = array('Property Name', 'Value');
    $rows = array();
    foreach ($properties as $property) {
      $rows[] = array(
        ucfirst(preg_replace('/_/', ' ', $property->type_id->name)),
        $property->value
      );
    }

    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_biosample-table-properties',
        'class' => 'tripal-data-table table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => '',
    );

    print theme_table($table);
  }


