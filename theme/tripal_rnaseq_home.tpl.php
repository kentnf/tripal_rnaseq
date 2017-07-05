<?php

?>

<div class="row"> <div class="col-md-12">
<h3>Method Description for RNA-seq</h3>
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

<h3>Method Description for small RNA-seq</h3>
<p>
The small RNA datasets were combined by removing redundancies (same sequences,
same lengths, and same directions). Small RNAs derived from rRNA and tRNA were
discarded. sRNAs <b>with transcripts per million (TPM) >=50 in at least one 
sample and with length of 19-24 nt</b> were included in the miRNA identification.
These sRNAs were aligned to the correspinding genome sequence and the flanking
sequences (200bp to each side) of sRNAs with no more than 20 unique genome hits
were extracted and folded with the <a href="http://rna.tbi.univie.ac.at" target="_blank">RNAfold</a> program.
The structures were then checked with <a href="http://bartellab.wi.mit.edu/software.html" target="_blank">miRcheck</a>
to identify potential miRNA candidates. miRNA candidates were then compared to
<a href="http://www.mirbase.org" target="_blank">miRBase</a> to identify
conserved miRNA candidates (up to two mismatches). The miRNA targets were
identified using the <a href="/mirtarget" target="_blank">target prediction tool</a>
developed and implemented in the database.
</p>

<h3>RNA-seq Collection</h3>
<p>The mRNA expression database contains samples collected from the following projects</p>
</div></div>
<div class="row">
<?php

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

// get type_id of sRNA experiment

$values = array(
  'name' => 'miRNA-seq',
  'is_obsolete' => 0,
  'cv_id' => array (
    'name' => 'experiment_strategy',
  ),
);
$result = chado_select_record('cvterm', array('cvterm_id', 'name'), $values);

if (empty($result)) {
  drupal_set_message("tripal_rnaseq: can not find type of miRNA-Seq", 'error');
}
$sRNA_type_id = $result[0]->cvterm_id;

// get all projects of rnaseult->species
$sql = "SELECT E.experiment_id, E.name, T.project_id, T.name as project_name, S.biomaterial_id,
     S.name as biomaterial_name, O.genus, O.species
    FROM chado.experiment E
    LEFT JOIN chado.biomaterial S ON E.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.organism O ON S.taxon_id = O.organism_id
    LEFT JOIN chado.project T ON E.project_id = T.project_id
    INNER JOIN chado.experimentprop P ON E.experiment_id = P.experiment_id
    WHERE P.type_id = :type_id
  ";
$args = array(':type_id' => $type_id);
$results = db_query($sql, $args)->fetchAll();

// save result to each organism
$org_project = array(
  'watermelon' => array(),
  'cucumber' => array(),
  'melon' => array(),
  'Cucurbita' => array(),
);

foreach ($results as $result) {
  if ($result->genus == 'Citrullus') {
    $org_project['watermelon'][] = l($result->project_name, 'rnaseq/wm/' . $result->project_id);
  }
  elseif ($result->species == 'sativus') {
    $org_project['cucumber'][] = l($result->project_name, 'rnaseq/cu/' . $result->project_id);
  }
  elseif ($result->species == 'melo') {
    $org_project['melon'][] = l($result->project_name, 'rnaseq/me/' . $result->project_id);
  }
  elseif ($result->genus == 'Cucurbita') {
    $org_project['Cucurbita'][] = l($result->project_name, 'rnaseq/pu/' . $result->project_id);
  }
  else {
    $unknown_org = $result->genus . " " . $result->species;
    drupal_set_message("tripal_rnaseq: unknown organism: $unknown_org", 'error');
  }
}

// display htm
foreach ($org_project as $general_name => $projects) {
  print "<div class=\"col-md-3\"><center>";
  $projects = array_unique($projects);
  print "<h4>" . ucfirst($general_name) . "</h4>";
  $image = '';
  if ($general_name == 'cucumber') {
    $image = "<img src=\"/sites/default/files/img/fp_cu_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  elseif ($general_name == 'watermelon') {
    $image = "<img src=\"/sites/default/files/img/fp_wm_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  elseif ($general_name == 'melon') {
    $image = "<img src=\"/sites/default/files/img/fp_me_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  else {
    $image = "<img src=\"/sites/default/files/img/fp_pu_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  print $image;

  foreach ($projects as $p) {
    print $p . "<br>";
  }

  print "</center></div>";
}

  // output result to table
  /**
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
  */

?>
</div>

