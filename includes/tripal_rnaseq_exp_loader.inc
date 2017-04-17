<?php

/**
 * load RNASeq Expression file into database 
 */ 

/**
 * submit a job for load experession file 
 * 
 * @param $node
 * 
 * generate job args, then trigger a tripal job
 */
function tripal_rnaseq_exp_load_job($node)
{
  global $user;
  if ($node->rnaseq_name) {
    $job_args = array(
      $node->rnaseq_path,
      $node->rnaseq_term,
      $node->bioproject_id,
    );
    tripal_add_job("Load Expression File: $node->rnaseq_path", 'tripal_rnaseq',
      'tripal_rnaseq_exp_loader', $job_args, $user->uid);
  }
}

/**
 * loading RNASeq expression
 */
function tripal_rnaseq_exp_loader($expression_file, $expression_term, $bioproject_id) {

  $transaction = db_transaction();
  print "\nNOTE: Loading RNASeq expression is performed using a database transaction. \n" .
     "If the load fails or is terminated prematurely then the entire set of \n" .
     "insertions/updates is rolled back and will not be found in the database\n\n";
  try {
    if (!is_readable($expression_file)) {
      watchdog('tripal_rnaseq_exp_loader',
        'Could not open the file :expression_file. Check that file exists and that permissions are correct .',
        array(':expression_file' => $expression_file),
        WATCHDOG_ERROR);
      exit;
    }

    // parse expression file and load to feature_expression table
    $num_seq = 0;
    $num_insert = 0;
    $num_update = 0;
    $sample = array();

    // check if the sample name is already exist in chado_experiment table
    // create array to story the sample name and experiment id
    $sample_exp = array();
    $fh = fopen($expression_file, 'r');
    if ($sample = fgetcsv($fh, 0, "\t")) {
      array_shift($sample);
      foreach ($sample as $sid) {
        $rr = db_query("SELECT experiment_id, project_id FROM chado.experiment WHERE name = :sid", array(':sid' => $sid))->fetchAll();
        if (sizeof($rr) == 0) {
          watchdog('tripal_rnaseq_exp_loader',
            'Sample name :sample does not exist in experiment table', array(':sample' => $sid), WATCHDOG_ERROR);
          exit;
        }
        $sample_exp[$sid] = $rr[0]->experiment_id; // link sample name ($sid) and experiment_id ($rr)

        // check if the BioProject is match (not required, but good for load/update expression for project) 
        if ($rr[0]->project_id != $bioproject_id) {
          watchdog('tripal_rnaseq_exp_loader',
            'Experiment :sample  does not belong to BioProject :project_id', array(':sample' => $sid, ':project_id'=> $bioproject_id), WATCHDOG_ERROR);
          exit;
        }
      }
    }

    while($line = fgetcsv($fh, 0, "\t")) 
    {
      // check feature name, must be exist feature in current database
      $feature_name = array_shift($line);
      $fr = chado_select_record('feature', array('feature_id'), array('uniquename' => $feature_name));
      if (sizeof($fr) != 1) {
        watchdog('tripal_rnaseq_exp_loader',
          'Can not find feature (Gene/mRNA) :feature from chado', array(':feature' => $feature_name), WATCHDOG_ERROR);
        exit;
      }
      $feature_id = $fr[0]->feature_id;

      $exp = array_combine($sample, $line); // convert two arrays to one key-value array

      foreach ($exp as $sample_name => $value) {
        $exp_id = $sample_exp[$sample_name];
        $sql = "SELECT feature_expression_id FROM chado.feature_expression
          WHERE feature_id = :feature_id AND experiment_id = :experiment_id";
        $args = array(':feature_id' => $feature_id, ':experiment_id' => $exp_id);
        $rr = db_query($sql, $args)->fetchAll();       

        //  update record for expression
        if (sizeof($rr) == 1) {
          $feature_exp_id = $rr[0]->feature_expression_id;

          $sql = "SELECT feature_expressionprop_id FROM chado.feature_expressionprop
          WHERE feature_expression_id = :feature_expression_id AND type_id = :type_id";
          $args = array(':feature_expression_id' => $feature_exp_id, 
            ':type_id' => $expression_term);
          $rrr = db_query($sql, $args)->fetchField(); 

          if ($rrr) {
            $feature_expprop_id = $rrr;
            // update expression value with expression_term
            $update_value = array(
              'value' => $value, //this value is expression value
            );

            $update = db_update('chado.feature_expressionprop')
              ->fields($update_value)
              ->condition('feature_expressionprop_id', $feature_expprop_id)
              ->execute();

            if (!$update) {
              watchdog('tripal_rnaseq_exp_loader',
              'Experiment : can not update record :feature_expressionprop_id', array(':feature_expressionprop_id' => $feature_expprop_id), WATCHDOG_ERROR);
              exit;
            }

          } else {
            // insert expression value with expression_term
            $exp_values = array(
              'feature_expression_id' => $rr[0]->feature_expression_id,
              'type_id' => $expression_term,
              'value' => $value,
            );
            $fexp_prop_id = db_insert('chado.feature_expressionprop')->fields($exp_values)->execute();
          } 
          ++$num_update;
        } 
        // insert new record for expression
        elseif (sizeof($rr) == 0) {
          // insert feature_expression
          $values = array(
            'feature_id' => $feature_id, 
            'experiment_id' => $exp_id,
            'pub_id'  => 0,
          );
          $feature_exp_id = db_insert('chado.feature_expression')->fields($values)->execute();

          // insert feature_expressionprop
          $exp_values = array(
            'feature_expression_id' => $feature_exp_id,
            'type_id' => $expression_term,
            'value' => $value, 
          );
          $fexp_prop_id = db_insert('chado.feature_expressionprop')->fields($exp_values)->execute();
          ++$num_insert;
        }
        // wrong, more record selected by feature_id and experiment_id
        else {
          watchdog('tripal_rnaseq_exp_loader',
            'Experiment :duplicate record exist in featrue_expression Feature :feature_id and Experiment :experiment_id', array(':experiment_id' => $exp_id, ':feature_id'=> $feature_id), WATCHDOG_ERROR);
          exit;
        }

        // report insert status
        ++$num_seq;
        print  "Insert RNASeq exp: " . $num_insert . ", Update sRNA exp: " . $num_update .
          ", RNASeq exp " . $num_seq . ". Memory: " .
          number_format(memory_get_usage()) . " bytes.\r";
      }
    }
    fclose($fh);
  }
  catch (Exception $e) {
    print "\n";
    watchdog_exception('tripal_rnaseq_exp_loader', $e);
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
  }
  print "\nDone.\n";
  return;
}


