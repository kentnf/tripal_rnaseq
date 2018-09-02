<?php
$bioproject = chado_generate_var('project', array('project_id' => $project_id));
$bioproject = chado_expand_var($bioproject, 'field', 'project.description');
?>
  <div class="tripal_project-data-block-desc tripal-data-block-desc"><h4>Overview of this BioProject:</h4></div>
  <?php

  $headers = array();
  $rows = array();

  // The bioproject name.
  $rows[] = array(
    array(
      'data' => 'BioProject',
      'header' => TRUE,
      'width' => '20%',
    ),
    '<i>' . $bioproject->name . '</i>'
  );

  // allow site admins to see the biomaterial ID
  if (user_access('view ids')) {
    // BioProject ID
    $rows[] = array(
      array(
        'data' => 'BioPorject ID',
        'header' => TRUE,
        'class' => 'tripal-site-admin-only-table-row',
      ),
      array(
        'data' => $bioproject->project_id,
        'class' => 'tripal-site-admin-only-table-row',
      ),
    );
  }

  // Generate the table of data provided above.
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_bioproject-table-base',
      'class' => 'tripal-bioproject-data-table tripal-data-table table',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // Print the table and the description.
  print theme_table($table);

  // Print the bioproject description.
  if ($bioproject->description) { ?>
    <div style="text-align: justify"><?php print $bioproject->description?></div> <?php
  }

  /**
   * display bioproject property
   */
  $bioproject = chado_expand_var($bioproject, 'table', 'projectprop', array('return_array' => 1));
  $properties = $bioproject->projectprop;

  // Check for properties.
  if (count($properties) > 0) {
    ?>
    <br>
    <div class="tripal_bioproject-data-block-desc tripal-data-block-desc"><h4>Properties this BioBroject:</h4></div>
    <?php

    $headers = array('Property Name', 'Value');
    $rows = array();
    $subprop = array();
    foreach ($properties as $property) {
      if ($property->type_id->cv_id->name == 'bioproject_property') {
        $subprop[$property->type_id->name] = $property->value;
      } else {
        $cv_name  = tripal_bioproject_cvterm_display($property->type_id->cv_id->name, 1);
        $cv_value = tripal_bioproject_cvterm_display($property->type_id->name, 1);

        if ($cv_value == 'Other') {
          if (!empty($subprop[$property->type_id->cv_id->name])) {
            $cv_value .=  ' (' . $subprop[$property->type_id->cv_id->name] . ')';
            unset($subprop[$property->type_id->cv_id->name]);
          }
        }

        $rows[] = array(
          ucfirst($cv_name),
          $cv_value,
        );
      }
    }

    foreach ($subprop as $name => $value) {
      $rows[] = array(
          ucfirst($name),
          $value,
      );
    }

    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_bioproject-table-properties',
        'class' => 'tripal-data-table table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => '',
    );
    print theme_table($table);
  }

  // show experiments related to this projects

  // get type_id of experiment
  $values = array(
    'name' => 'RNA-Seq',
    'is_obsolete' => 0,
    'cv_id' => array (
      'name' => 'experiment_strategy',
    ),
  );
  $result = chado_select_record('cvterm', array('cvterm_id', 'name'), $values);

  if (empty($result)) {
    drupal_set_message("tripal_rnaseq: can not find type of RNA-seq", 'error');
  }
  $type_id = $result[0]->cvterm_id;

  // get all RNASeq experiment from bioproject
  $sql = "SELECT EX.experiment_id, EX.name as exp_name, EX.description, 
      S.biomaterial_id, S.name as sample_name, O.genus, O.species, O.common_name FROM chado.experiment EX
    LEFT JOIN chado.biomaterial S ON EX.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.organism O ON S.taxon_id = O.organism_id
    INNER JOIN chado.experimentprop P ON EX.experiment_id = P.experiment_id
    WHERE P.type_id = :type_id AND project_id = :bioproject_id";
  $experiments = db_query($sql, array(':bioproject_id'=> $project_id, ':type_id'=>$type_id))->fetchAll();;

  if (sizeof($experiments) > 0) {
    ?>
    <br>
    <div class="tripal_bioproject-data-block-desc tripal-data-block-desc"><h4>BioSamples of this BioProject:</h4></div>
    <?php

    $headers = array('BioSample', 'Organism', 'Description');
    $rows = array();
    $enames = array();
    foreach ($experiments as $experiment) {
      if ($experiment->genus == 'Citrullus' and $general_org == 'wm') {
      }
      elseif ($experiment->species == 'sativus' and $general_org == 'cu') {
      }
      elseif ($experiment->species == 'melo' and $general_org == 'me') {
      }
      elseif ($experiment->genus == 'Cucurbita' and $general_org == 'pu') {
      }
      else {
        //continue; // skip if the experiment and general_org does not match
      }

      // get prop of biosample for description
      $values = array(
         'biomaterial_id' => $experiment->biomaterial_id
      );
      $biosample = chado_generate_var('biomaterial', $values); 
      $biosample = chado_expand_var($biosample, 'table', 'biomaterialprop');
      $description = $biosample->description . ";  ";
      foreach ($biosample->biomaterialprop as $prop) {
        $cvterm = '<a href=# data-toggle="tooltip" title="'.$prop->type_id->definition.'">'.$prop->type_id->name.'</a>';
        $description .= $cvterm . " : " . $prop->value . ";  ";
      } 

      $rows[] = array(
		l($experiment->sample_name, 'rnaseqsample/' . $experiment->biomaterial_id),
		$experiment->common_name,
        $description,
      );
      $enames[] = $experiment->exp_name;
      #dpm($experiment);
    }

    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal_experiment-table-properties',
        'class' => 'tripal-data-table table'
      ),
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => array(),
      'empty' => '',
    );
    print theme_table($table);
  }

  $_SESSION['tripal_rnaseq_analysis']['samples'] = $enames;

