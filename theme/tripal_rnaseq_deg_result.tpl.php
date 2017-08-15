<?php

/**
 * Display the results of a DEG job execution
 */

// dpm($tripal_args);

// display information for DEG analysis
$project_id  = $_SESSION['tripal_rnaseq_analysis']['project_id'];
$organism_id = $_SESSION['tripal_rnaseq_analysis']['organism_id']; 

$bioproject = chado_generate_var('project', array('project_id' => $project_id));
$bioproject = chado_expand_var($bioproject, 'field', 'project.description');

$organism = chado_generate_var('organism', array('organism_id'=>$organism_id));
$organism_common_name = l($organism->common_name, 'organism/' . $organism_id);

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

?>
  <p>
    <b>Reference Geome:</b> <?php print $organism_common_name ?> <br>

    <?php if ($tripal_args['type'] == 'p') { ?>
      <b>Sample A:</b> <?php print $tripal_args['sampleA'] ?> <br>
      <b>Sample B:</b> <?php print $tripal_args['sampleB'] ?> <br>
    <?php } else { ?>
      <b>Samples:</b> <?php print $tripal_args['sampleC'] ?> <br>
    <?php } ?>

    <b>DEG Analysis Program:</b> <?php print $tripal_args['rscript'] ?> <br>

    <?php if ($tripal_args['type'] == 'p') { ?>
    <b>Ratio cutoff:</b> <?php print $tripal_args['ratio'] ?> <br>
    <?php } ?>

    <b>Adjust P-value cutoff:</b> <?php print $tripal_args['adjp'] ?> <br>
    <b>No. of significantly chagned genes:</b> <?php print $deg_num ?> <br>
    <b>*Note</b>: only top <b><font color=red><?php print $display_num ?></font></b> significantly changed genes show in below table, or you can<br>
  </p>
<?php

// display result table for DEG analysis 
if ($deg_table) {

    // display links for downstream analyis
    if (sizeof($deg_table) > 0) {
      $output = $tripal_args['output'];
      $output_all = $output;
      $output_all = preg_replace('/deg\.txt/', 'deg.all.txt', $output_all);
      ?>
      <p><b>Download full list of <?php print $deg_num ?> significantly changed genes:</b> 
        <a href="<?php print '../../' . $output; ?>">Tab-delimited Format</a> <br>
       
        <b>Download DEG analysis result for all genes:</b>
        <a href="<?php print '../../' . $output_all; ?>">Tab-delimited Format</a> <br>
      </p>

      <p><b>Avaiable analyses for significantly changed genes:</b> <br>
        <a href=/goenrich>GO Ernichment</a> |
        <a href=/funcat>GO Slim Gene Classifiction</a> |
        <a href=/pwyenrich>Pathway Ernichment</a> |
        <a href=/batchquery data-toggle="tooltip" title="retrive sequence, function annotation, or family information fo the significantly changed genes">Batch Query</a>
      </p>
      <br>
      </div></div>

      <?php
    }

    // display DEG tables
    $header_data = array_shift($deg_table);
    $rows_data = array();
    foreach ($deg_table as $line) 
    {
      $gene_id = $line[0];
      $line[0] = l($gene_id, 'feature/gene/' . $gene_id, array('attributes' => array('target' => "_blank"))); 
      $rows_data[] = $line;
    }
 
    $header = array(
      'data' => $header_data,
    );
    $rows = array(
      'data' => $rows_data,
    );
    $variables = array(
      'header' => $header_data,
      'rows' => $rows_data,
      'attributes' => array('class' => 'table table-striped table-bordered', 'id'=>'degtable'),
    );
    print theme('table', $variables);
}
else {
  ?><div>No result</div><?php 
}

