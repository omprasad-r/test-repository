<?php

// Do not scan the init module dir for drush command files.
$options['ignored-modules'][] = 'acsf_init';
$options['ignored-modules'][] = 'migrate';
