<?php

class AcsfEventHandlerIncompatibleException extends Exception {}

/**
 * An event within the ACSF framework encapsulates a dispatcher and a list of event
 * handlers. The event will contain an internal context that is accessible from
 * the handlers.
 *
 * $type = 'site_duplication_scrub';
 * $registry = acsf_get_registry();
 * $context = array('key' => 'value');
 * $event = new AcsfEvent(
 *   new AcsfEventDispatcher(),
 *   new AcsfLog(),
 *   $type,
 *   $registry,
 *   $context);
 * $event->run();
 */

class AcsfEvent {

  protected $handlers;

  /**
   * Constructor.
   *
   * @param AcsfEventDispatcher $dispatcher
   *   The event dispatcher object.
   * @param AcsfLog $log
   *   The log object.
   * @param String $type
   *   The type of event to run.
   * @param Array $registry
   *   The registry from acsf_registry.
   * @param Array $context
   *   An arbitrary context for handlers.
   * @param AcsfSite $site
   *   The site being operated upon (optional).
   */
  public function __construct(AcsfEventDispatcher $dispatcher, AcsfLog $log, $type, $registry, $context, AcsfSite $site = NULL) {

    $this->dispatcher = $dispatcher;
    $this->log = $log;
    $this->type = $type;
    $this->site = $site;
    $this->registry = $registry;
    $this->context = $context;
    $this->handlers = array(
      'incomplete' => array(),
      'complete' => array(),
      'failed' => array(),
    );
  }

  /**
   * Creates an event using ACSF defaults.
   *
   * @param String $type
   *   The type of event to execute.
   * @param Array $context
   *   A custom context to pass to event handlers.
   */
  public static function create($type, $context) {
    $registry = acsf_get_registry();
    $event = new static(
      new AcsfEventDispatcher(),
      new AcsfLog(),
      $type,
      $registry,
      $context);

    return $event;
  }

   /**
    * Produces data that can be used to debug an event and the performance of its
    * handlers including run time and error messages.
    */
   public function debug() {
     $debug = array();
 
     foreach (array_keys($this->handlers) as $key) {
       foreach ($this->handlers[$key] as $handler) {
         $debug['handlers'][$key][] = array(
          'class' => get_class($handler),
          'started' => $handler->started,
          'completed' => $handler->completed,
          'message' => $handler->message,
         );
       }
     }

     return $debug;
   }

  /**
   * Loads event handlers for the appropriate event.
   */
  protected function loadHandlers() {
    foreach ($this->registry['events'] as $module => $info) {
      if ($info['type'] == $this->type) {
        $path = trim($info['path'], '/');
        $class = $info['class'];
        require sprintf('%s/%s/%s.inc', DRUPAL_ROOT, $path, $class);
        $this->pushHandler(new $class($this), 'incomplete');
      }
    }
  }

  /**
   * Pops (actually shifts to preserve order) a handler from the internal list.
   *
   * @param String $type
   *   The type of handler: incomplete, complete or failed.
   *
   * @return AcsfEventHandler
   */
  public function popHandler($type = 'incomplete') {
    if (array_key_exists($type, $this->handlers)) {
      return array_shift($this->handlers[$type]);
    }
    else {
      throw new AcsfEventHandlerIncompatibleException(sprintf('The handler type "%s" is incompatible with this event.', get_class($handler)));
    }
  }

  /**
   * Pushes a handler to in internal list.
   *
   * @param AcsfHandler $handler
   *   The handler to add.
   * @param String $type
   *   The type of handler: incomplete, complete or failed.
   */
  public function pushHandler($handler, $type = 'incomplete') {
    if (!is_subclass_of($handler, 'AcsfEventHandler')) {
      throw new AcsfEventHandlerIncompatibleException(sprintf('The handler class "%s" is incompatible with this event, must subclass AcsfEventHandler.', get_class($handler)));
    }

    if (array_key_exists($type, $this->handlers)) {
      $this->handlers[$type][] = $handler;
    }
    else {
      throw new AcsfEventHandlerIncompatibleException(sprintf('The handler type "%s" is incompatible with this event.', $type));
    }
  }

  /**
   * Dispatches all event handlers.
   */
  public function run() {
    $this->loadHandlers();
    $this->dispatcher->dispatch($this);
  }

}
