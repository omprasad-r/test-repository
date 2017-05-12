<?php

/**
  * @file
  * Pre-settings-php hook to increase the memory conditionally on few user pages.
  * This hook has been added to fix the tokens not loading across the sites as per insight acquia tickets-379840.
  * Example from: https://docs.acquia.com/article/conditionally-increasing-memory-limits
  *
  */

 $url = $_SERVER['SERVER_NAME'];
 $str_arr = explode('.',$url);
  
  if ( 
	($str_arr[0] =="http://vivoconcerti") || ($str_arr[0] =="https://vivoconcerti") || ($str_arr[0] =="vivoconcerti") ){ 
	  if (
		(strpos($_GET['q'], 'user') === 0) || (strpos($_GET['q'], 'user/register') === 0) || (strpos($_GET['q'], 'user/password') === 0) ||
		(strpos($_GET['q'], 'user/') === 0 && (preg_match('/^user\/[\d]+\/edit/', $_GET['q']) === 1))
	   ) {
	  ini_set('memory_limit', '128M');
	} 
  }	
 