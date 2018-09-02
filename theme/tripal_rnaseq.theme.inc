<?php

function tripal_rnaseq_preprocess_tripal_rnaseq_deg_result(&$variables) {

  //Reference https://datatables.net/examples/styling/bootstrap.html
  $path = drupal_get_path('module', 'tripal_rnaseq');
  //drupal_add_js($path . '/theme/js/jquery-1.12.4.js', array('type'=>'file'));
  drupal_add_js($path . '/theme/js/jquery.dataTables.min.js', array('type'=>'file'));
  drupal_add_js($path . '/theme/js/dataTables.bootstrap.min.js', array('type'=>'file'));
    $javascript = "
      jQuery(document).ready(function() {
        jQuery('#degtable').DataTable();
      } );
    ";
    drupal_add_js($javascript, array('type'=>'inline','weight' => 50));
  drupal_add_css($path . '/theme/dataTables.bootstrap.min.css/jquery.dataTables.min.js', array('type'=>'file'));

  $tripal_job = tripal_get_job($variables['job_id']); 
  $tripal_args = unserialize($tripal_job->arguments);
  $variables['tripal_job']  = $tripal_job;
  $variables['tripal_args'] = $tripal_args;

  $deg_file = DRUPAL_ROOT . DIRECTORY_SEPARATOR . $tripal_args['output'];
  $num = 0;
  $num_cutoff = 100;

  // put output table result to array
  if (is_readable($deg_file)) {
    $deg_table = array();
    $genes_str = '';
    $fp = fopen($deg_file,'r');
    if (($headers = fgetcsv($fp, 0, "\t")) !== FALSE) {
      if ($headers) {
        $deg_table[] = $headers;
        while (($line = fgetcsv($fp, 0, "\t")) !== FALSE) {
          if ($line) {
            $num++;
            if ($num <= $num_cutoff) {
              $padj = array_pop($line);
              array_push($line, sprintf('%.2e', $padj));
              $deg_table[] = $line;
            }
            $genes_str.= $line[0] . "\n";
          }
        }
      }
    }
    fclose($fp);
    $variables['deg_table'] = $deg_table;
    $variables['deg_num'] = $num;
    $variables['display_num'] = $num_cutoff;
    $_SESSION['tripal_rnaseq_analysis']['genes_str'] = $genes_str; 
  }
}

function tripal_rnaseq_preprocess_tripal_rnaseq_heatmap(&$variables) {

  //dpm($variables);
  $exp_json = json_encode($variables['exp']);
  $data_js = "
    var expression = $exp_json;
  ";

  $path = drupal_get_path('module', 'tripal_rnaseq');
  drupal_add_js($path . '/theme/js/d3.min.js', array('type'=>'file'));
  drupal_add_js($path . '/theme/js/plotly-latest.min.js', array('type'=>'file'));

  drupal_add_js($data_js, array('type'=>'inline', 'scope'=>'footer','weight' => 50));
  drupal_add_js($path . '/theme/js/tripal_rnaseq_heatmap_plotly.js', array('type'=>'file', 'scope'=>'footer','weight' => 100));
}

function tripal_rnaseq_preprocess_tripal_feature_rnaseq_expression(&$variables) {

  $path = drupal_get_path('module', 'tripal_rnaseq');
  drupal_add_js($path . '/theme/js/d3.min.js', array('type'=>'file'));
  drupal_add_js($path . '/theme/js/plotly-latest.min.js', array('type'=>'file'));
 
  // retrieve RNASeq expression dataset for this feature
  $feature_id = $variables['node']->feature->feature_id;
  $sql = "SELECT EXP.value, 
      C.name as type_name,
      P.project_id, P.name as project_name, 
      S.biomaterial_id, S.name as biosample_name,
      E.experiment_id, E.name as sample_name
    FROM chado.feature_expressionprop EXP
    LEFT JOIN chado.feature_expression EX ON EXP.feature_expression_id = EX.feature_expression_id
    LEFT JOIN chado.cvterm C ON EXP.type_id = C.cvterm_id
    LEFT JOIN chado.experiment E ON EX.experiment_id = E.experiment_id
    LEFT JOIN chado.biomaterial S ON E.biomaterial_id = S.biomaterial_id
    LEFT JOIN chado.project P ON E.project_id = P.project_id
    WHERE 
      EX.feature_id = :feature_id
  ";

  $args = array(':feature_id' => $feature_id);

  // put RNASeq expression to rnaseq_exp
  $results = db_query($sql, $args)->fetchAll();
  /**
   * data structure;
   * rnaseq_exp [array]
   *   project_id [obj]
   *     name  = project_name
   *     RPM [array]
   *       sample_id1 = value1
   *       sample_id2 = value2
   *       ...
   *     RPKM [array]
   *     raw_count [array]
   *     [other project properties]
   *      
   *     design [obj]

   * samples [array] 
   *   sample_id [obj]
   *     name = sample_name
   *     factor1 = factor1_name    eg: treatment = Ethylene
   *     factor2 = factor2_name    eg: stage = 10DAA
   *     factorN = factorN_name    eg: tissue = leaf
   *     [other sample properties]
   */

  if (sizeof($results) > 0) {
    $rnaseq_exp = array(); // key project id, 
    $samples = array();  // key sample id,
    $samples_name_id = array();
    foreach ($results as $r) {
      $type = $r->type_name;
      $project_id = $r->project_id;
      $project_name = $r->project_name;
      $type = $r->type_name;
      $sample_id = $r->experiment_id;
      $sample_name = $r->sample_name;
      $value = $r->value;

      if (!array_key_exists($project_id, $rnaseq_exp)) {
        $rnaseq_exp[$project_id] = new stdClass;
        if (!isset($rnaseq_exp[$project_id]->name)) {
          $rnaseq_exp[$project_id]->name = $project_name;
          $rnaseq_exp[$project_id]->project_id = $project_id;
        }
      }

      if (!property_exists($rnaseq_exp[$project_id], $type)) {
        $rnaseq_exp[$project_id]->$type = array();
      }
      $rnaseq_exp[$project_id]->$type[$sample_id] = $value;

      if (!array_key_exists($sample_id, $samples)) {
        $samples[$sample_id] = new stdClass;
        if (!isset($samples[$sample_id]->name)) {
          $samples[$sample_id]->name = $sample_name;
          $samples_name_id[$sample_name] = $sample_id;
        }
      }
    }

    $variables['experiments'] = $samples;
    $variables['rnaseq_exp'] = $rnaseq_exp;
    //$variables['design'] = $design; 

    // retrieve experiment design for each project
    foreach ($rnaseq_exp as $project_id) {
      $values =  array(
        'project_id' => $project_id->project_id,
        'type_id' => array (
          'cv_id' => array (
             'name' => 'bioproject_property',
           ),
           'name' => 'experimental_design',
           'is_obsolete' => 0
        ),
      );
      $result = chado_select_record('projectprop', array('value'), $values);
      if (!empty($result)) {
        $description = array();
        $lines = explode("\n", $result[0]->value);
        $title = explode(" ", preg_replace("!\s+!", " ", array_shift($lines)));
        foreach ($lines as $line) {
          if (preg_match("/^#/", $line)) {
            $description[] = $line;
            continue;
          }
          $member = explode(" ", preg_replace("!\s+!", " ", $line));
          $sample_name = $member[0];
          $sample_id = $samples_name_id[$sample_name];

          for($i=1; $i<count($member); ++$i) {
            if (isset($member[$i]) and isset($title[$i])) {
              $factor = $title[$i];
              $factor_name = $member[$i];
              $samples[$sample_id]->$factor = $factor_name;
            }
          }
        }

        // generate designs according to factors
        $factors = $title; 
        array_shift($factors);
        $designs = array();
        $design_num = 0;
        foreach ($factors as $f1) {
          foreach ($factors as $f2) {
            if ($f1 == $f2) {
              continue;
            }
            $design_num++;
            $designs[$design_num] = array($f1, $f2);
          }
        }
			
        if (sizeof($factors) > 0) {
          $rnaseq_exp[$project_id->project_id]->designs = $designs;
        }
        if (sizeof($description) > 0) {
          $rnaseq_exp[$project_id->project_id]->desc = $description;
        }
      }
    }

    // convert the experiments and rnaseq_exp to json format
    $experiments_json = json_encode($samples);
    $rnaseq_exps_json = json_encode($rnaseq_exp);

    $data_js = "
      var experiments = $experiments_json;
      var rnaseq_exps = $rnaseq_exps_json;
    ";
    drupal_add_js($data_js, array('type'=>'inline', 'scope'=>'footer','weight' => 50));
    drupal_add_js($path . '/theme/js/tripal_rnaseq_histogram_plotly.js', array('type'=>'file', 'scope'=>'footer','weight' => 100));    
  }
}
