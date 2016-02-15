<?php
/**
 * @file
 * Context Forms & Handler.
 */

/**
 * Expose themes as context reactions.
 */
class environment_libraries_context_reaction_library extends context_reaction {

  /**
   *
   */
  public function sortLibraries($l1, $l2) {
    return strcmp($l1, $l2);
  }

  /**
   * Provide libraries to load for this context.
   */
  public function options_form($context) {
    // ## Define our Libraries context form.
    // Load all of our environment_libraries_library entities.
    $libraries = environment_libraries_library_load_multiple();
    $available_libraries = array();
    foreach ($libraries as $library) {
      $available_libraries[$library->machine_name] = t($library->title);
    }
    // Sort the libraries by name keyed by id.
    uasort($available_libraries, array('environment_libraries_context_reaction_library', 'sortLibraries'));
    // Get current values.
    $values = $this->fetch_from_context($context);
    // Create a list of checkboxes.
    $form['environment_libraries'] = array(
      '#title' => t('Environment Libraries'),
      '#description' => t('Include JS & CSS Libraries.'),
      '#type' => 'checkboxes',
      '#options' => $available_libraries,
      '#default_value' => isset($values['environment_libraries']) ? $values['environment_libraries'] : array(),
    );

    // Add a link to manage libraries.
    $content['environment_libraries_manage'] = array(
      '#theme' => 'link',
      '#text' => 'Manage Libraries',
      '#path' => 'admin/config/system/environment_libraries',
      '#options' => array(
        'attributes' => array(),
        'html' => TRUE,
      ),
    );

    return $form;
  }

  /**
   * Options form submit handler.
   */
  public function options_form_submit($values) {
    return $values;
  }

  /**
   * Add the libraries for this context to the page.
   */
  public function execute() {
    // ## When a context is active.
    foreach ($this->get_contexts() as $k => $v) {
      if (!empty($v->reactions[$this->plugin]['environment_libraries'])) {
        $libraries = $v->reactions[$this->plugin]['environment_libraries'];
        foreach ($libraries as $index => $enabled) {
          // Load it's enabled libraries.
          if (!$enabled) {
            continue;
          }
          if (is_numeric($index)) {
            // Allow lid index for backward compatibility.
            $library = environment_libraries_library_load(intval($index));
          }
          else {
            $library = environment_libraries_library_load_by_machine_name($index);
          }
          if ($library !== FALSE) {
            // Make sure the library exists before we try and load it.
            environment_libraries_load($library->module ?: 'environment_libraries', $library->machine_name, $library->environment);
          }
        }
      }
    }
  }

}
