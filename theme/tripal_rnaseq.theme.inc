<?php


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

    dpm($samples_name_id);

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
       $matrix_str = preg_replace("!\s+!", " ", $result[0]->value);
       dpm($matrix_str);
       $matrix = str_getcsv($matrix_str, " ", '' , "\n");
       dpm($matrix);

       $rnaseq_exp[$project_id->project_id]->design = $result[0]->value;
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
