<?php

// clean the rnaseq session
unset($_SESSION['tripal_rnaseq_analysis']);

?>

<div class="row"> <div class="col-md-12">
<h3>RNA-Seq analysis method description</h3>
<p>

The raw RNA-Seq reads were processed to remove adapters as well as low quality bases using
<a href="http://www.usadellab.org/cms/?page=trimmomatic" target="_blank">Trimmomatic</a>, 
and the trimmed reads shorter than 80% of original length were discarded. The remaining high-quality reads were aligned to the 
<a href="https://www.arb-silva.de/" target="_blank">SILVA rRNA database</a> 
to remove rRNA sequences
using <a href="bowtie-bio.sourceforge.net" target="_blank">Bowtie</a> allowing up to two 
mismatches. The resulting reads/read pairs were aligned to the corresponding genome using 
<a href="https://ccb.jhu.edu/software/hisat" target="_blank">HISAT</a> allowing up to two mismatches. 
The expression levels of transcripts were measured and normalized to FPKM (fragments per kilobase 
of exon per million mapped fragments) base on all mapped reads.
</p>

<h3>RNA-Seq collections</h3>
<p>The cucurbit expression atlas database contains the following projects</p>
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
/**
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
*/

// get all projects of rnaseult->species
$sql = "SELECT E.experiment_id, E.name, T.project_id, T.name as project_name, T.description, S.biomaterial_id,
     S.name as biomaterial_name, O.genus, O.species, O.common_name
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

  $limit = 100;
  $text = '';
  if (str_word_count($result->description, 0) > $limit) {
          $words = str_word_count($result->description, 2);
          $pos = array_keys($words);
          $text = substr($result->description, 0, $pos[$limit]) . '...';
  } else {
    $text = $result->description;
  }

  $tooltip = array(
    'attributes' => array(
      'data-toggle'=>'tooltip', 
      'title'=> $text
    )
  );

  if ($result->genus == 'Citrullus') {
    $org_project['watermelon'][] = l($result->project_name, 'rnaseq/wm/' . $result->project_id, $tooltip);
  }
  elseif ($result->species == 'sativus') {
    $org_project['cucumber'][] = l($result->project_name, 'rnaseq/cu/' . $result->project_id, $tooltip);
  }
  elseif ($result->species == 'melo') {
    $org_project['melon'][] = l($result->project_name, 'rnaseq/me/' . $result->project_id, $tooltip);
  }
  elseif ($result->genus == 'Cucurbita') {
    $org_project['Cucurbita'][] = l($result->project_name, 'rnaseq/pu/' . $result->project_id, $tooltip);
  }
  else {
    $sname = preg_replace('/ \(.*/', '', $result->common_name);
	$org_project[$sname][] = l($result->project_name, 'rnaseq/other/' . $result->project_id, $tooltip);
    //$unknown_org = $result->genus . " " . $result->species;
    //drupal_set_message("tripal_rnaseq: unknown organism: $unknown_org", 'error');
  }
}

// display html
print "<div class=\"row\">";
$order = 0;
foreach ($org_project as $general_name => $projects) {
  $order++;
  $projects = array_unique($projects);

  print "<div class=\"col-md-3\"><center>";
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
  elseif ($general_name == 'Cucurbita') {
    $image = "<img src=\"/sites/default/files/img/fp_pu_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  else {
    $image = "<img src=\"/sites/default/files/img/bg_s.png\" style=\"height:75px;width:85px\" /><br><br>";
  }
  print $image;

  foreach ($projects as $p) {
    print $p . "<br>";
  }

  print "</center></div>";

  if ($order % 4 == 0) {
	print "</div>";
	print "<div class=\"row\">";
  }
}

print "</div>";

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

