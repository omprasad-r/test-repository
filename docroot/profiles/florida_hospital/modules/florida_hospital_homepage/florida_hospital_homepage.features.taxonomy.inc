<?php
/**
 * @file
 * florida_hospital_homepage.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function florida_hospital_homepage_taxonomy_default_vocabularies() {
  return array(
    'page_categories' => array(
      'name' => 'Page Categories',
      'machine_name' => 'page_categories',
      'description' => 'Filtering categories for pages.',
      'hierarchy' => '0',
      'module' => 'taxonomy',
      'weight' => '0',
      'metatags' => array(),
      'rdf_mapping' => array(
        'rdftype' => array(
          0 => 'skos:ConceptScheme',
        ),
        'name' => array(
          'predicates' => array(
            0 => 'dc:title',
          ),
        ),
        'description' => array(
          'predicates' => array(
            0 => 'rdfs:comment',
          ),
        ),
      ),
    ),
  );
}
