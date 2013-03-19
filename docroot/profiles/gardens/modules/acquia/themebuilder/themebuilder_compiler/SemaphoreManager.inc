<?php

/**
 * The age of a semaphore that is considered stale.
 */
define('THEMEBUILDER_COMPILER_SEMAPHORE_MAX_AGE', 60*5);

/**
 * The minimum age of a semaphore that we will consider for removal.
 */
define('THEMEBUILDER_COMPILER_SEMAPHORE_MIN_AGE', 5);

/**
 * The key of the semaphore used to delete semaphores when we have run out.
 */
define('THEMEBUILDER_COMPILER_DELETE_SEMAPHORE', 37);

/**
 * This class is responsible for getting a list of existing semaphores,
 * finding aged semaphores, and deleting semaphores from the system.
 * Semaphores are system resources that are allocated in the kernel.  Once
 * they are created, you have to remove them or you will run out.
 */
class SemaphoreManager {
  private $user;

  /**
   * The user that owns this process.
   */
  static $currentUser = NULL;

  /**
   * Sets the user.
   *
   * This value is used when deleting semaphores.  We will only delete
   * semaphores for the specified user.
   *
   * @param {String} $user
   *   The user.
   */
  public function setUser($user) {
    $this->user = $user;
  }

  /**
   * Returns the user.
   *
   * If the user was not set using the setUser command, the current
   * user will be retrieved using the 'whoami' command.
   *
   * @return {String}
   *   The user.
   */
  public function getUser() {
    if (isset($this->user)) {
      $user = $this->user;
    }
    else {
      if (NULL === self::$currentUser) {
        self::$currentUser = exec('whoami');
      }
      $user = self::$currentUser;
    }
    return $user;
  }

  /**
   * Acquires the semaphore associated with the specified semaphore key.
   *
   * The key must be a 32 bit integer.  This call will block until the
   * semaphore becomes available.
   *
   * @param {int} $key
   *   The semaphore key.  This key is supposed to be as unique as
   *   possible in the set of semaphores you are likely to request.
   *   Each process that wants to access the same singleton resources
   *   must always acquire the semaphore using the same key.
   *
   * @return {Resource}
   *   The resource that represents the semaphore.  This resource will
   *   subsequently be passed to the release method to release the
   *   semaphore.
   */
  public function acquire($key) {
    if (!is_int($key)) {
      throw new ThemeBuilderException("SemaphoreManager::acquire called with a non-integer key ($key)");
    }
    $status = FALSE;
    $hasDeleteSemaphore = FALSE;
    do {
      try {
        if (!isset($deleteSemaphore)) {
          // This is important.  Make sure that we get a reference to
          // the delete semaphore (don't acquire it though).  If we
          // run into a situation where we need to delete semaphores,
          // it will be necessary to acquire the delete semaphore
          // which will be impossible if the semaphore doesn't already
          // exist.  In that case we would have to remove one or more
          // semaphores without the benefit of having the delete
          // semaphore, meaning that multiple processes could do
          // exactly the same thing at the same time.
          $deleteSemaphore = $this->getDeleteSemaphore();
        }
        $resource = @sem_get($key);
        if (!$resource) {
          // Failed to get the semaphore.  This is most likely due to the system
          // running out of semaphores.  Last I checked there were 128 possible
          // semaphores on the system.
          if (!$hasDeleteSemaphore) {
            // Do not release this semaphore until we have been able
            // to acquire the desired semaphore.  Otherwise multiple
            // php processes will get interleaved access to the delete
            // semaphore, making it impossible to credibly honor
            // request order.  This should be released at the end of
            // the method.
            $hasDeleteSemaphore = $this->acquireDeleteSemaphore();
          }
          
          // Semaphores may have been removed while waiting to acquire
          // the delete semaphore by other php processes executing
          // this same code.
          $resource = @sem_get($key);
          if (!$resource) {
            // We were not able to get a reference to the desired
            // semaphore.  Usually this means the maximum number of
            // semaphores has been reached.  In this case we have to
            // remove some of them (carefully!) before trying to get
            // the semaphore again.
            $this->deleteAgedSemaphores();
            $resource = @sem_get($key);
          }
        }
        if ($resource) {
          $status = @sem_acquire($resource);
          // Note that there are 2 scenarios in which the sem_acquire
          // function returns. 1) The semaphore has been released.  2)
          // The semaphore has been removed.  If the semaphore is
          // removed, the sem_acquire function echos an error message
          // into the log and returns FALSE.  At that point the code
          // must do a sem_get to get a resource that references the
          // newly-established semaphore.
        }
      }
      catch (Exception $e) {
      }
    } while ($status != TRUE);
    if ($hasDeleteSemaphore) {
      $this->releaseDeleteSemaphore();
      $hasDeleteSemaphore = FALSE;
    }
    if ($status != TRUE) {
      throw new ThemeBuilderException("Failed to acquire semaphore");
    }
    return $resource;
  }

  /**
   * Releases the semaphore associated with the specified resource.
   *
   * If the semaphore release fails, it could be due to the semaphore
   * being removed after it was acquired by this process.  Either way
   * it isn't catastrophic if you don't release it because it will be
   * released automatically when this process terminates.
   *
   * @param {Resource} $resource
   *   The resource associated with the semaphore to release.
   *
   * @return {boolean}
   *   TRUE if the semaphore was released successfully; FALSE otherwise.
   */
  public function release($resource) {
    return @sem_release($resource);
  }

  /**
   * Releases and removes a semaphore that is currently acquired by this process.
   *
   * The semaphore will only be removed if there is no pending
   * interest in the semaphore.  In the case that it cannot be
   * removed, the semaphore will be released as a result of calling
   * this method.
   *
   * @param {String} $key
   *   The semaphore key.
   * @param {Resource} $resource
   *   The resource returned from sem_get().
   *
   * @return
   *   TRUE if the semaphore has been removed; FALSE otherwise.
   */
  public function remove($key, $resource) {
    $result = FALSE;
    $semaphore = $this->getSemaphoreInfo($key);
    if ($this->okToRemoveAcquired($semaphore)) {
      $result = @sem_remove($resource);
    }
    else {
      // We were not able to remove the semaphore.  We should release
      // the semaphore since we clearly don't want it any more.
      $this->release($resource);
    }
    return $result;
  }

  /**
   * Deletes stale semaphores that are owned by the user associated with this SemaphoreManager instance.
   *
   * This method requires that the delete semaphore is acquired by
   * this process.  If the delete semaphore has not been acquired, it
   * will both be acquired and released as a result of calling this
   * method.  Otherwise it is assumed that the caller will acquire and
   * release the semaphore.
   *
   * @return {int}
   *   The number of semaphores deleted as a result of calling this method.
   */
  public function deleteAgedSemaphores() {
    // Make sure we have the delete semaphore before trying to delete
    // anything.
    $releaseSemaphore = FALSE;
    if (empty($this->deleteSemaphore)) {
      $releaseSemaphore = TRUE;
      $this->acquireDeleteSemaphore();
    }

    $age = THEMEBUILDER_COMPILER_SEMAPHORE_MAX_AGE;
    $count = 0;
    $now = time();
    $okToExit = FALSE;
    do {
      $semaphores_to_delete = $this->getAgedSemaphores($age);
      try {
        $count = $this->deleteSemaphores($semaphores_to_delete);
      }
      catch (Exception $e) {
      }
      if ($count <= 0) {
        // We failed to identify possible semaphores to delete.
        // Expand the search.
        $age = $this->changeStaleAge($age);
      }

      // Getting the semaphore involves executing an external program.
      // We don't want to do that every time through this loop, but
      // the number of semaphores will change over time so we have to
      // do it occasionally.  Check the number of semaphores every 2
      // seconds while we are trying to delete semaphores.
      if ((time() - $now) > 2) {
        $totalSemaphores = $this->getSemaphoreCount();
        $now = time();
        if ($totalSemaphores < 100) {
          // We should be able to create a semaphore.  Avoid an
          // infinite loop in the case in which no semaphores can be
          // deleted because we don't actually own any.
          $okToExit = TRUE;
        }
      }
      // Only exit if there are plenty of semaphores available or if
      // we were able to free some semaphores.
    } while(!$okToExit && (!isset($count) || $count <= 0));

    // Only release the delete semaphore if we acquired it in this
    // method.
    if ($releaseSemaphore === TRUE) {
      $this->releaseDeleteSemaphore();
    }
    return $count;
  }

  /**
   * Returns an array of semaphores older than the specified age.
   *
   * @param {int} $age
   *   The age of the semaphores we are looking for, measured in seconds.
   *
   * @return {array}
   *   The list of semaphores.
   */
  public function getAgedSemaphores($age) {
    $semaphores = $this->getMySemaphores();
    $now = time();
    
    $aged = array();
    foreach ($semaphores as $index => $semaphore) {
      if ($semaphore->last_op !== 0) {
        $sem_age = $now - $semaphore->last_op;
        if ($sem_age >= $age) {
          $aged[] = $semaphore;
        }
      }
    }
    return $aged;
  }

  /**
   * Returns the number of semaphores currently in existence.
   *
   * @return {int}
   *   The number of semaphores.
   */
  public function getSemaphoreCount() {
    return count($this->getAllSemaphores());
  }

  /**
   * Gets all semaphores owned by the user associated with this
   * SemaphoreManager instance.
   *
   * The user should be set using the setUser method, otherwise the
   * user that owns the php process will be used.
   *
   * @return {array}
   *  The array of objects representing semaphores.
   */
  public function getMySemaphores() {
    $mySemaphores = array();
    $semaphores = $this->getAllSemaphores();
    $count = count($semaphores);
    $user = $this->getUser();

    // Only add semaphores owned by the user to the resulting array.
    for ($i = 0; $i < $count; $i++) {
      if ($semaphores[$i]->owner === $user) {
        $mySemaphores[] = $semaphores[$i];
      }
    }
    return $mySemaphores;
  }

  /**
   * Returns an array of all semaphores.
   *
   * @return {Array}
   *   The array of semaphores.
   */
  public function getAllSemaphores() {
    $semaphores = array();
    $output = array();
    exec('ipcs -st', $output);
    $semaphores = $this->getObjectsFromTable($output, 'semid', $this->getIpcsRegex('-st'));
    return $semaphores;
  }

  /**
   * Returns an object that represents a semaphore from the semaphore map.
   *
   * @param {int} $key
   *   The key associated with the desired semaphore.  Either the key
   *   or the id is required to uniquely identify the semaphore, but
   *   not both.
   * @param {String} $id
   *   The semaphore id associated with the desired semaphore.
   *
   * @return {stdClass}
   *   An object representing the semaphore, or NULL if the semaphore
   *   could not be found.
   */
  public function getSemaphoreFromMap($key = NULL, $id = NULL) {
    $tries = 0;
    $maxTries = 2;
    $map = $this->getSemaphoreMap();
    do {
      if (!empty($key) && isset($map->keyMap[$key])) {
        $semaphore = $map->keyMap[$key];
      }
      elseif (!empty($id) && isset($map->idMap[$id])) {
        $semaphore = $map->idMap[$id];
      }
      if (!isset($semaphore)) {
        $map = $this->getSemaphoreMap(TRUE);
      }
      $tries++;
    } while (!isset($semaphore) && $tries < $maxTries);
    if (!isset($semaphore)) {
      // We failed to get the semaphore info because the semaphore
      // does not exist.
      $semaphore = NULL;
    }
    return $semaphore;
  }

  /**
   * Returns semaphore information for the semaphore identified by
   * either the specified key or the specified value.
   *
   * @param {int} $key
   *   The key that identifies the semaphore.
   * @param {String} $id
   *   The semaphore id.
   *
   * @return {stdClass}
   *   An object containing fields that represent the information
   *   associated with the semaphore.
   */
  public function getSemaphoreInfo($key = NULL, $id = NULL) {
    $semaphore = $this->getSemaphoreFromMap($key, $id);
    if (NULL == $semaphore) {
      return $semaphore;
    }

    // Grab the output of running 'ipcs -si'.
    $output = array();
    exec('ipcs -si ' . escapeshellarg($semaphore->semid), $output);
    $info = $this->getObjectsFromTable($output, 'semnum', $this->getIpcsRegex('-si'));
    if (!isset($semaphore->data)) {
      $semaphore->data = array();
    }
    $count = count($info);
    for ($i = 0; $i < $count; $i++) {
      $semaphore->data[$info[$i]->semnum] = $info[$i];
    }
    return $semaphore;
  }

  /**
   * Returns true if the specified semaphore is currently in the acquired state.
   *
   * @param {Object} $semaphore
   *   An object representing the semaphore, returned from the
   *   getSemaphoreInfo method.
   *
   * @return {boolean}
   *   TRUE if the specified semaphore is in the acquired state or if
   *   it should be ignored because it is not a semaphore controlled
   *   by a php process; FALSE otherwise.
   */
  public function isAcquired($semaphore) {
    if (count($semaphore->data) == 3) {
      return $semaphore->data[0]->value == 0;
    }
    // This semaphore was not established by a PHP process, so we wish
    // to ignore it for the purposes of deleting semaphores.
    return TRUE;
  }

  /**
   * Returns true if the specified semaphore currently has processes waiting to acquire it.
   *
   * @param {Object} $semaphore
   *   An object representing the semaphore, returned from the
   *   getSemaphoreInfo method.
   *
   * @return {int}
   *   The number of processes waiting to acquire the specified semaphore
   */
  public function pendingProcesses($semaphore) {
    if (count($semaphore->data) == 3) {
      return $semaphore->data[0]->ncount;
    }
    return -1;
  }

  /**
   * Returns TRUE if the specified semaphore currently has processes
   * that are interested in it.  This interest is registered in the
   * form of calling sem_get(key) to get the semaphore resource.
   *
   * @param {Object} $semaphore
   *   An object representing the semaphore, returned from the
   *   getSemaphoreInfo method.
   *
   * @return {int}
   *   The number of processes that have registered interest in the
   *   specified semaphore.
   */
  public function interestedProcesses($semaphore) {
    if (count($semaphore->data) == 3) {
      return $semaphore->data[1]->value;
    }
    return -1;
  }
  
  /**
   * Implements the policy with respect to when it is ok to remove a semaphore that this process already has acquired.
   *
   * NOTE: This process should only use this method if the specified
   * semaphore is already in the acquired state by this process.
   * Otherwise use the okToRemove method.
   *
   * @param {Object} $semaphore
   *   An object representing the semaphore, returned from the
   *   getSemaphoreInfo method.
   *
   * @return {boolean}
   *   TRUE if it is ok to remove this semaphore; FALSE otherwise.
   */
  public function okToRemoveAcquired($semaphore) {
    if ($semaphore->key != THEMEBUILDER_COMPILER_DELETE_SEMAPHORE && isset($semaphore) && isset($semaphore->data)) {
      if ($this->isAcquired($semaphore) && $this->pendingProcesses($semaphore) == 0 && $this->interestedProcesses($semaphore) == 1) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Determines whether it is ok to remove the specified semaphore
   * given that this process has not acquired the specified semaphore
   * first.  In this case we want to make sure that the semaphore is
   * not currently acquired and has no pending interest.
   *
   * @param {Object} $semaphore
   *   An object representing the semaphore, returned from the
   *   getSemaphoreInfo method.
   *
   * @return
   *  TRUE if it is fine to remove the semaphore; FALSE otherwise.
   */
  public function okToRemove($semaphore) {
    if ($semaphore->key != THEMEBUILDER_COMPILER_DELETE_SEMAPHORE && isset($semaphore) && isset($semaphore->data)) {
      if (!$this->isAcquired($semaphore) && $this->pendingProcesses($semaphore) == 0 && $this->interestedProcesses($semaphore) == 0) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Retrieves the mapping between semaphore keys and semaphore ids.
   *
   * This map allows us to look up basic semaphore information based
   * on either the semaphore key or the semaphore id.
   *
   * @param {boolean} $refresh
   *   If FALSE, the cached data will be used; otherwise the data will
   *   be gathered again.
   *
   * @return {stdClass}
   *   An object containing a 'keyMap' field that is useful for
   *   looking up semaphore information by key and an 'idMap' field
   *   that is useful for looking up semaphore info by semaphore id.
   */
  public function getSemaphoreMap($refresh = FALSE) {
    if ($refresh === TRUE || !isset($this->map)) {
      $output = array();
      $result = new stdClass();
      $result->keyMap = array();
      $result->idMap = array();

      // Grab the semaphore information.
      exec('/usr/bin/ipcs -s ', $output);
      $semaphores = $this->getObjectsFromTable($output, 'key', $this->getIpcsRegex('-s'));

      $count = count($semaphores);
      for ($i = 0; $i < $count; $i++) {
        // Construct a lookup table that allows us to access this
        // data efficiently from either the semaphore key or the
        // semaphore id.
        $result->keyMap[hexdec($semaphores[$i]->key)] = $semaphores[$i];
        $result->idMap[$semaphores[$i]->semid] = $semaphores[$i];
      }
      $this->map = $result;
    }
    return $this->map;
  }

  /**
   * Prints the specified array of semaphores.
   *
   * @param {Array} $semaphores
   *   The array of objects, each of which represents a semaphore.
   */
  public function printSemaphores($semaphores) {
    foreach ($semaphores as $index => $semaphore) {
      $this->printSemaphore($semaphore);
    }
  }

  /**
   * Prints the specified semaphore.
   *
   * @param {stdClass} $semaphore
   *   The object that represents a semaphore.
   */  
  public function printSemaphore($semaphore) {
    print('id: ' . $semaphore->semid
      . ' owner: ' . $semaphore->owner
      . ' last_op: ' . date('c', $semaphore->last_op)
      . ' last_changed: ' . date('c', $semaphore->last_changed)
      . "\n");
  }

  /**
   * Returns a reference to the resource representing the semaphore
   * used to delete semaphores.  It is important that all semaphore
   * deletions occur only when this resource has been acquired so that
   * multiple processes can work in a coordinated fashion.
   *
   * Note that this method does not actually acquire the delete
   * semaphore; it only gets a reference to the underlying resource.
   * Use acquireDeleteSemaphore if you need to actually delete
   * semaphores.
   *
   * @return {Resource}
   *   A reference to the resource that represents the delete semaphore.
   */
  private function getDeleteSemaphore() {
    $age = THEMEBUILDER_COMPILER_SEMAPHORE_MAX_AGE;
    do {
      try {
        $del = @sem_get(THEMEBUILDER_COMPILER_DELETE_SEMAPHORE);
        if (!$del) {
          // Unable to get the delete semaphore reference.  We will
          // have to forcibly remove a semaphore.  This should happen
          // only once on each webnode provided we establish the
          // semaphore and never delete it.  By having each process
          // that requires a semaphore to first get the delete
          // semaphore, enough interest will be registered against the
          // delete semaphore that our delete logic would be able to
          // avoid removing it.
          $semaphores_to_delete = $this->getAgedSemaphores($age);
          $count = count($semaphores_to_delete);

          // We have a set of semaphores that may be appropriate for
          // deletion.  The goal of this method is to get the delete
          // semaphore and we were not able to do that.  Here we want
          // to delete the minimum number of semaphores possible to
          // guarantee that we get a reference to the delete semaphore
          // resource.  Nothing more.  Note that this deletion is not
          // gated by the delete semaphore, so we want the smallest
          // footprint possible here.
          for ($i = 0; $i < $count && !$del; $i++) {
            try {
              $semaphore = $this->getSemaphoreInfo(NULL, $semaphores_to_delete[$i]->semid);
              if ($semaphore && $this->okToRemove($semaphore)) {
                exec('ipcrm -s ' .escapeshellarg($semaphores_to_delete[$i]->semid));
                $del = @sem_get(THEMEBUILDER_COMPILER_DELETE_SEMAPHORE);
              }
            }
            catch (Exception $infoEx) {
            }
          }
        }
        if (!$del) {
          // We failed to identify possible semaphores to delete.
          // Expand the search.
          $age = $this->changeStaleAge($age);
        }
      } catch (Exception $e) {
      }
    } while (!$del);
    return $del;
  }

  /**
   * Acquires the semaphore used when we have to forcibly delete
   * semaphores.  This is used to try to ensure multiple processes are
   * not deleting semaphores at the same time, as deleting a semaphore
   * that is in use has destructive effects.
   *
   * @return {boolean}
   *   TRUE if the delete semaphore was acquired; FALSE otherwise.
   *   Note that this method will continue to try to acquire the
   *   delete semaphore, so it will never actually return FALSE,
   *   though it could take some time to actually acquire the
   *   semaphore.
   */
  private function acquireDeleteSemaphore() {
    $acquired = FALSE;
    do {
      try {
        $del = $this->getDeleteSemaphore();
        if ($del) {
          $acquired = @sem_acquire($del);
          // The flow of execution will continue if either the
          // semaphore is acquired or if the semaphore has been
          // removed, making $del a reference to a non-existent
          // resource.  If the semaphore has not been acquired, it is
          // important to attempt to get a reference to the semaphore
          // resource again, or we will have an infinite loop.
        }
      } catch (Exception $e) {
      }
    } while (!$acquired);
    $this->deleteSemaphore = $del;
    return TRUE;
  }

  /**
   * Releases the delete semaphore.
   */
  private function releaseDeleteSemaphore() {
    if (!empty($this->deleteSemaphore)) {
      @sem_release($this->deleteSemaphore);
      // sem_release returns FALSE on failure, but if we can't release
      // the semaphore there is really nothing to do.  The semaphore
      // may have been removed by an external process, for example.
      // At any rate the semaphore will be released when the php
      // process exits.
      unset($this->deleteSemaphore);
    }
  }

  /**
   * Deletes the semaphores in the specified array.
   *
   * This method does a safe deletion in which the semaphore will not
   * be deleted if it is currently in the acquired state or if another
   * process has registered interest in the semaphore.
   *
   * @param {Array} $semaphores
   *   An array of objects, each representing a semaphore to delete.
   *
   * @return {int}
   *   The number of semaphores that were deleted.
   */
  public function deleteSemaphores($semaphores) {
    if (!is_array($semaphores)) {
      throw new Exception('Argument passed to SemaphoreManager::deleteSemaphores is not an array.');
    }
    $releaseSemaphore = FALSE;
    if (empty($this->deleteSemaphore)) {
      $releaseSemaphore = TRUE;
      $this->acquireDeleteSemaphore();
    }
    $count = count($semaphores);
    $ids = array();

    // Only add semaphores that have no pending interest to the list
    // to be deleted.
    for ($i = 0; $i < $count; $i++) {
      $semaphore = $this->getSemaphoreInfo(NULL, $semaphores[$i]->semid);
      if ($semaphore && $this->okToRemove($semaphore)) {
        $ids[] = $semaphore->semid;
      }
    }

    // Delete the semaphores.
    if (count($ids) > 0) {
      exec('ipcrm -s ' . implode(' -s ', $ids));
    }

    // Only release the delete semaphore if we acquired it in this
    // method.
    if ($releaseSemaphore === TRUE) {
      $this->releaseDeleteSemaphore();
    }
    return count($ids);
  }

  /**
   * Creates an array of objects representing a table of data
   * contained within the specified lines of text.
   *
   * The tabular data must be preceded by a line representing the
   * fields that should be set into the object.
   *
   * @param {Array} $lines
   *   An array of strings representing the contents to be parsed to
   *   get the data.
   * @param {String} $legendPrefix
   *   A string that identifies the legend that precedes the actual data.
   * @param {String} $regex
   *   The regular expression used to separate the fields.
   *
   * @return {Array}
   *   An array of objects, each of which represents a row of data.
   */
  private function getObjectsFromTable($lines, $legendPrefix, $regex) {
    $result = array();
    $length = count($lines);

    // Find the line that contains the legend.
    for ($i = 0; $i < $length && strpos($lines[$i], $legendPrefix) !== 0; $i++);

    if ($i < $length) {
      // Found the line containing the legend.  This legend will
      // represent our field names.
      $legendIndex = $i;
      $legend = $this->fixLegend($this->parseLine($lines[$legendIndex], $regex));

      // Now read in the data, constructing an object repreesnting each line.
      for ($i = $legendIndex + 1; $i < $length; $i++) {
        $data = $this->parseLine($lines[$i], $regex);
        if (count($data) === count($legend)) {
          $obj = $this->constructObj($legend, $data);
          $result[] = $obj;
        }
      }
    }
    return $result;
  }

  /**
   * Parses a line of text, returning an array of information found in
   * the line.  The line must have fields separated by whitespace and
   * not contain whitespace within any field.
   *
   * @param {String} $line
   *   The line of text to parse.
   *
   * @return {Array}
   *   An array of data representing the fields found in the specified line.
   */
  private function parseLine($line, $regex) {
    $result = array();
    $matches = array();
    preg_match($regex, $line, $matches);
    $length = count($matches);
    for ($i = 1; $i < $length; $i++) {
      $result[] = $matches[$i];
    }
    return $result;
  }

  /**
   * This method modifies the array that represents the legend of tabular data so the field names can be used as object fields.
   *
   * The values are trimmed and names containing characters that are
   * illegal in instance variable names are modified.
   *
   * @param {Array} $legend
   *   The legend.
   *
   * @return {Array}
   *   The modified legend.
   */
  private function fixLegend($legend) {
    $count = count($legend);
    for ($i = 0; $i < $count; $i++) {
      $legend[$i] = trim(str_replace('-', '_', $legend[$i]));
    }
    return $legend;
  }

  /**
   * Creates an object with fields from the specified legend and
   * values from the specified data array.
   *
   * @param {Array} $legend
   *   The legend for the data.  Each element in the legend array is a
   *   string representing the name of the object field into which the
   *   respective value will be placed.
   * @param {Array} $data
   *   The data.  Each element in the data array is a string
   *   representing a value that will be placed into the resulting
   *   object.
   *
   * @return {stdClass}
   *   The object.
   */
  private function constructObj($legend, $data) {
    $length = min(count($legend), count($data));
    $result = new stdClass();
    for ($i = 0; $i < $length; $i++) {
      if (isset($legend[$i]) && strlen($legend[$i]) > 0 && isset($data[$i]) && strlen($data[$i]) > 0) {
        $result->$legend[$i] = $data[$i];
      }
    }
    return $result;
  }

  /**
   * Returns a regular expression that can be used to separate data for the output of ipcs when used with the specified arguments.
   *
   * @param {String} $arg
   *   The arguments being passed to the ipcs command.
   *
   * @return {String}
   *   A regular expression appropriate for separating the data
   *   resulting in the ipcs execution.
   */
  private function getIpcsRegex($arg) {
    switch ($arg) {
      case '-s':
      case '-si':
        $regex = '/^\s*([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*$/';
        break;

      case '-st':
        $regex = '/^(\S*)[ ]+(\S+)[ ]+(\S.*) [ ]+(\S.*)$/';
        break;

      default:
        throw new Exception("Cannot provide a regex for unknown ipcs args $args.");
    }
    return $regex;
  }

  /**
   * Returns an appropriate semaphore age that would constitute staleness for the purpose of removing semaphores to make room for new requests.
   *
   * This method is used to reduce the amount of time since the last
   * semaphore activity that would represent a stale semaphore.
   * Semaphores are a limited resource, so they must be reaped at some
   * point.  When a request to get a new semaphore fails, this object
   * will attempt to remove some to make more room.
   *
   * The age starts at 5 minutes
   * (THEMEBUILDER_COMPILER_SEMAPHORE_MAX_AGE), and if we find that no
   * semaphores can be removed, we reduce the age that represents
   * staleness until we can remove at least one semaphore.
   *
   * This function honors the minimum age, which is defined as
   * THEMEBUILDER_COMPILER_SEMAPHORE_MIN_AGE.
   *
   * @param {int} $age
   *   The current age representing staleness, measured in seconds.
   *
   * @return {int}
   *   The new stale age.
   */
  private function changeStaleAge($age) {
    $age = max(THEMEBUILDER_COMPILER_SEMAPHORE_MIN_AGE, intval($age / 2));
    return $age;
  }
}
