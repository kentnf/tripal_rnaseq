<?php
  $bioproject = chado_generate_var('project', array('project_id' => $project_id));
  $bioproject = chado_expand_var($bioproject, 'field', 'project.description');

  ?>
  <div class="row"> <div class="col-md-10 col-md-offset-1">
  <?php
  
  $bioproject_url = l($bioproject->name, '/bioproject/' . $project_id);
  $sra_url = '';

  if (preg_match("/^PRJ/", $bioproject->name)) { 
    $sra_url = l('link to SRA', 'https://www.ncbi.nlm.nih.gov/bioproject/' . $bioproject->name, array('attributes' => array('target'=>'_blank'))); 
  }
  ?><div><b>Bioproject: </b> <?php print $bioproject_url ." &nbsp;&nbsp;&nbsp; ". $sra_url ?> </div><?php


  if ($bioproject->description) { ?>
    <div style="text-align: justify"><b>Project Description:</b> <br><?php print $bioproject->description?></div><br> <?php
  }

  // get sample description
  $bioproject = chado_expand_var($bioproject, 'table', 'projectprop');
  $design_value = null;

  foreach ($bioproject->projectprop as $property) {
    if ($property->type_id->name == 'experimental_design') {
      $design_value = $property->value;
      break;
    }
  }

  $design_sample_value = array();

  if (isset($design_value)) {
    $lines = explode("\n", $design_value);
    foreach ($lines as $line) {
      $line = trim($line);
      if (preg_match("/^#/", $line) && strlen($line) > 2) {
        $design_sample_value[] = $line;
      }
    }
  }

  if (sizeof($design_sample_value) > 0){
    ?><div style="text-align: justify"><b>Sample Description:</b> <br><?php
    foreach ($design_sample_value as $sample_desc) {
      print $sample_desc . '<br>';
    }
    ?></div><br><?php
  }
?>
  <p><b>Avaiable analyses for Selected Genes:</b> <br> 
    <a href=/goenrich>GO Ernichment</a> | 
    <a href=/funcat>GO Slim Gene Classifiction</a> | 
    <a href=/pwyenrich>Pathway Ernichment</a> |
    <a href=/batchquery>Batch Query</a> 
  </p>
  <div id=tripal-rnaseq-heatmap></div>
</div></div>
