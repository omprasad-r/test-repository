<?php

// Make sure it is not cached by Varnish.
header("Cache-Control: no-cache");

//$sitename = $_GET["sitename"];
$sitename = $_SERVER["SERVER_NAME"];
$themes_path = "./sites/" . $sitename . "/themes/mythemes/";
if(isset($_GET["themedir"])){
  $themedir = $_GET["themedir"];
  $themedir_path = $themes_path . $themedir . "/";
  $infofile = $themedir_path . $themedir . ".info";
} 
$operation = $_GET["operation"];

function list_contents($directory) {
    //Scan the directory's contents
    if (is_dir($directory)) {
        $iter = new DirectoryIterator($directory);
        foreach ($iter as $file) {
            //And print the content
            print $file . "\n";
        }
    } else {
        print "Error: " . $directory . " is not a directory";
    }
}

function rmdir_rec($dir, $skipFiles=array('.', '..')) {
    $dir .= ( substr($dir, 0, -1) == '/') ? '' : '/';
    foreach (scandir($dir) as $file) {
        if (in_array($file, $skipFiles) === false) {
            if (is_dir($dir . $file))
                rmdir_rec($dir . $file);
            (is_dir($dir . $file)) ? rmdir($dir . $file) : unlink($dir . $file);
        }
    }
}

function switch_themebuilder_dev_mode($sitename, $switchto) {
    $drush_path = shell_exec('which drush');
    if (empty($drush_path)) {
      // might happen on local installs
      $out =  system("../drush/drush -l http://" . $sitename . " pm-" . $switchto . " themebuilder_test --yes", $retval);
    } else {
      $out =  system("drush -l http://" . $sitename . " pm-" . $switchto . "  themebuilder_test --yes", $retval);
    }
    if ( $out === false ){
      print "System failed while executing drush sql-dump...";
    }
    elseif ( $retval !== 0 ){
      print "System call to drush returned non-zero exit status...";
    }
    else print "\nDone!";
}

//#TODO: MAKE THIS ONE WORK ON GSTEAMER (FILE PERMISSIONS)

function enable_dev_mode($sitename) {
    print "Enabling development mode for site '" . $sitename . "':\n";
    $file = "./sites/" . $sitename . "/settings.php";
    if (file_exists($file)) {
        print "File " . $file . "doesn't exist.\n";
    } else {
        // The new person to add to the file
        $dev_mode_line = "\$conf['acquia_gardens_developer_mode'] = TRUE;\n";
        // Write the contents to the file, 
        // using the FILE_APPEND flag to append the content to the end of the file
        // and the LOCK_EX flag to prevent anyone else writing to the file at the same time
        file_put_contents($file, $dev_mode_line, FILE_APPEND | LOCK_EX);
        print "Done, modified" . $file . "'\n";
    }
}

function upload_image($image_from, $image_to) {
    //Get the image as a stream
    $contents_from = file_get_contents($image_from);
    if (false === ($contents_from)) {
        print "Error: failed to open image file: " . $image_from;
    } else {
        //And write the stream back out to file
        if (false === file_put_contents($image_to, $contents_from)) {
            print "Error: failed to put downloaded image here: " . $image_to;
        } else {
            print 'Uploaded image to session!';
        }
    }
}

function unpack_module($zip_file, $module_path) {
    $zip = new ZipArchive();
    //Open the archive
    if (false !== $zip->open($zip_file)) {
        //Extract the archive
        if (false !== $zip->extractTo($module_path)) {
            print "Extracted " . $zip_file . " to " . $module_path;
        } else {
            print "Error: failed to extract " . $zip_file . " to " . $module_path;
        }
        //Close the archive
        $zip->close();
        //And remove the .zip file
        if (false !== unlink($zip_file)) {
            print "File " . $zip_file . " removed.";
        } else {
            print "Error: failed to remove file " . $zip_file;
        }
    } else {
        print "Error: failed to unzip archive " . $zip_file;
    }
}

function take_dump($dump_file) {
    $drush_path = shell_exec('which drush');
    if (empty($drush_path)) {
      $out = system("../drush/drush sql-dump --result-file=" . $dump_file , $retval);
    } else {
      $out = system("drush sql-dump --result-file=" . $dump_file, $retval);
    }
    if ( $out === false ){
      print "System failed while executing drush sql-dump...";
    }
    elseif ( $retval !== 0 ){
      print "System call to drush returned non-zero exit status...";
    }
    else print "\nDone taking dump!";
}


switch ($operation) {
    case "get_hostname":
        print php_uname('n');
        break;
    case "list_themes":
        print "Listing themes located at " . $themes_path . "\n";
        list_contents($themes_path);
        break;
    case "list_themefolder":
        if (empty($themedir)) {
            print "No 'themedir' parameter";
        } else {
            print "Listing contents of " . $themedir_path . "\n";
            list_contents($themedir_path);
        }
        break;
    case "list_themefolder_images":
        if (empty($themedir)) {
            print "No 'themedir' parameter";
        } else {
            print "Listing contents of " . $themedir_path . "/images\n";
            list_contents($themedir_path . "/images");
        }
        break;
    case "enabledevmode":
        enable_dev_mode($sitename);
        break;
    case "enabletbdevmode":
        switch_themebuilder_dev_mode($sitename, "enable");
        break;
    case "disabletbdevmode":
        switch_themebuilder_dev_mode($sitename, "disable");
        break;
    case "infofile":
        if (empty($themedir)) {
            print "No 'themedir' parameter";
        } else {
            print "Reading " . $infofile;
            readfile($infofile);  //print readfile() is unnecessary, readfile specification indicates that file is output to Output Buffer...
        }
        break;
    case "upload_image_to_session":
        $image_to = $_GET["img_to"];
        $image_from = $_GET["img_from"];
        print "Uploading image " . $image_from . " to " . $image_to . ".\n";
        upload_image($image_from, $image_to);
        break;
    case "module_upload":
        if ($_FILES["file"]["error"] > 0) {
            print "Error: " . $_FILES["file"]["error"];
        } else {
            $file_name = $_FILES["file"]["name"];
            $temp_file = $_FILES["file"]["tmp_name"];
            $module_root = $_SERVER['DOCUMENT_ROOT'] . "/sites/" . $sitename . "/modules/";
	    // create dir recursively if it doesn't exist
            if (!file_exists($module_root)) {
                mkdir($module_root, 0755, TRUE);
            }
            $zip_file = $module_root . $file_name;

            print "File Uploaded to modules directory: " . $file_name . "(Size: " . ($_FILES["file"]["size"] / 1024) . "Kb";
            if (false !== move_uploaded_file($temp_file, $zip_file)) {
                unpack_module($zip_file, $module_root);
            } else {
                print "Error: failed to move " . $temp_file . " to " . $zip_file;
            }
        }
        break;
    case "module_delete":
        $module_folder = $_GET["module_name"];
        if (empty($module_folder)) {
            print "Empty module_name parameter";
        } else {
            $module_folder_path = $_SERVER['DOCUMENT_ROOT'] . "/sites/" . $sitename . "/modules/" . $module_folder . "/";
            rmdir_rec($module_folder_path);
            print "Done, deleted " . $module_folder_path;
        }
        break;
   case "db_dump":
      $dumpfile = $_GET["dumpfile"];
      take_dump($dumpfile);
      break;
}
?>