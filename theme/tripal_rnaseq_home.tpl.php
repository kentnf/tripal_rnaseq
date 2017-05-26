<?php

?>

<h3>The mRNA-seq page is under construction. </h3>

<div class="row"> <div class="col-md-8 col-md-offset-2">
<h3>Method Description</h3>
<p>

The raw RNA-Seq reads were processed to remove adapters as well as low quality bases using
<a href="http://www.usadellab.org/cms/?page=trimmomatic" target="_blank">Trimmomatic</a>, 
and the trimmed reads shorter than 80% of original length were discarded. The remaining high
quality reads were subjected to rRNA sequence removal by aligning to the 
<a href="https://www.arb-silva.de/" target="_blank">SILVA rRNA database</a>
using <a href="bowtie-bio.sourceforge.net" target="_blank">Bowtie</a> allowing up to two 
mismatches. The resulting reads/read pairs were aligned to the corresponding genome using 
<a href="https://ccb.jhu.edu/software/hisat" target="_blank">HISAT</a> allowing up to two mismatches. 
The expression of transcripts was measured and normalized to FPKM (fragments per kilobase 
of exon per million mapped fragments) base on all mapped read.
</p>

<h3>mRNA-seq Collection</h3>
<p>The mRNA expression database contains samples collected from the following projects</p>

<?php

// get all organism
$organisms = tripal_sra_get_organism_select_options();

foreach ($organisms as $organism_id => $common_name) {

  // get project, sample, and experiment for this organism
  $values = array(
    'name' => 'RNA-Seq',
    'is_obsolete' => 0,
    'cv_id' => array (
      'name' => 'experiment_strategy',
    ),
  );
  $result = chado_select_record('cvterm', array('cvterm_id', 'name'), $values);

  if (empty($result)) {
    drupal_set_message("tripal_srna: can not find type_od of miRNA-Seq", 'error');
  }
  $type_id = $result[0]->cvterm_id;

  // get small RNA experiment
  $sql = "SELECT E.experiment_id, E.name, T.project_id, T.name as project_name, S.biomaterial_id, 
     S.name as biomaterial_name 
    FROM chado.experiment E
    LEFT JOIN chado.biomaterial S ON E.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.project T ON E.project_id = T.project_id
    INNER JOIN chado.experimentprop P ON E.experiment_id = P.experiment_id
    WHERE P.type_id = :type_id AND S.taxon_id = :organism_id 
  ";
  $args = array(':type_id' => $type_id, ':organism_id' => $organism_id);
  $result = db_query($sql, $args)->fetchAll();

  // output result to table
  if (sizeof($result) > 0) {
    $header_data = array('Project', 'Sample', 'Description');
    $rows_data = array();
    foreach ($result as $exp) {
      $project = l($exp->project_name, 'bioproject/' . $exp->project_id);
      $sample = l($exp->biomaterial_name, 'biosample/' . $exp->biomaterial_id);
      $biomaterial = chado_generate_var('biomaterial', array('biomaterial_id' => $exp->biomaterial_id));
      $biomaterial = chado_expand_var($biomaterial, 'table', 'biomaterialprop', array('return_array' => 1));
      $desc = '';
      foreach ($biomaterial->biomaterialprop as $prop) {
        $desc.= $prop->type_id->name . " : " . $prop->value . "; ";
      }
      $rows_data[] = array($project, $sample, $desc);
    }

    $variables = array(
      'header' => $header_data,
      'rows' => $rows_data,
      'attributes' => array('class' => 'table vertical-align'),
    );
    print "<h4>$common_name</h4>";
    print theme('table', $variables);
  }
}

?>
</div></div>

