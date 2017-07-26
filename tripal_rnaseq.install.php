<?php
/**
 * @file
 * Functions used to install the module
 */

/**
 * Implements install_hook().
 *
 * Permforms actions when the module is first installed.
 *
 * @ingroup tripal_rnaseq
 */
function tripal_rnaseq_install() {
  tripal_create_files_dir('tripal_rnaseq');
  tripal_create_files_dir('tripal_rnaseq_exp');
  tripal_rnaseq_add_cvterms();
}

/**
 * add cvterms for expression value
 */
function tripal_rnaseq_add_cvterms() {

  $expression_term = array(
    'raw_count' => 'raw count number of reads',
    'RPKM' => 'Reads Per Kilobase of transcript per Million', 
    'FPKM' => 'Fragments Per Kilobase of transcript per Million',
    'RPM'  => 'Reads per Million',
	'SD_raw_count' => 'SD for error bar of raw count',
    'SD_RPKM' => 'SD for error bar of RPKM',
    'SD_FPKM' => 'SD for error bar of FPKM',
    'SD_RPM'  => 'SD for error bar of RPM'
  );

  tripal_insert_cv(
    'tripal_rnaseq',
    'Contains property terms for tripal rnaseq.'
  );

  foreach ($expression_term as $term => $description) {
    tripal_insert_cvterm(array(
      'name' => $term,
      'definition' => $description,
      'cv_name' => 'tripal_rnaseq',
      'db_name' => 'tripal_sra',
    ));
  }
}

/** 
 * Implements hook_schema()
 */
function tripal_rnaseq_schema() {
  // table for store loaded expression file as node, which used for display in gene feature page 
  $schema['chado_rnaseq'] = array(
    'description' => t('The table for RNASeq node'),
    'fields' => array(
      'nid' => array(
        'description' => t('The primary identifier for a node.'),
        'type' => 'serial', 'unsigned' => true, 'not null' => true,
      ),
      'name' => array(
        'description' => t('The human-readable name for experession file.'),
        'type' => 'varchar', 'length' => 1023, 'not null' => true,
      ),
      'path' => array(
        'description' => t('The full path of the expression file.'),
        'type' => 'varchar', 'length' => 1023, 'not null' => true,
      ),
      'type_id' => array(
        'description' => t('The type of the expression value.'),
        'type' => 'int', 'size' => 'big', 'not null' => true,
      ),
      'project_id' => array(
        'description' => t('the project id.'),
        'type' => 'int', 'size' => 'big', 'not null' => true,
      ),
      'organism_id' => array(
        'description' => t('the organism id.'),
        'type' => 'int', 'size' => 'big', 'not null' => true,
      ),
    ),
    'indexes' => array(
      'name' => array('name'),
    ),
    'primary key' => array('nid'),
    'unique keys' => array(
      'nid' => array('nid'),
      'type_project_id' => array('type_id', 'project_id', 'organism_id'),
    ),
  );

  return $schema;
} 

/**
 * Implements hook_update(),
 * 
 * update chado.feature_expression
 * change the field name from expression_id to expreriment_id
 * FOREIGN KEY (experiment_id) REFERENCES expression(expression_id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED
 * FOREIGN KEY (experiment_id) REFERENCES experiment(experiment_id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED
 *
 *  drop the constraint "feature_expression_pub_id_fkey" on "feature_expression"?
 *
 * can not do it, already did it manually 
 */

function tripal_rnaseq_update_1000() {

  // Changing the length of the type field to allow it to be more readable.
  //db_change_field('chado.feature_expression', 'expression_id', 'experiment_id',
  //  array(
  //      'description' => t('link it to experiment table of tripal_sra module.'),
  //  )
  //);
}

