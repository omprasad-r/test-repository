<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title><?php echo $title; ?></title>
    <link rel="stylesheet" type="text/css" href="<?php echo $path_to_jsunit; ?>/css/jsUnitStyle.css">
    <script type="text/javascript" src="<?php echo $path_to_jsunit; ?>/app/jsUnitCore.js"></script>
    <script type="text/javascript">

        function coreSuite() {
            var result = new JsUnitTestSuite();
            <?php echo $javascript; ?>
            return result;
        }

        function suite() {
            var newsuite = new JsUnitTestSuite();
            newsuite.addTestSuite(coreSuite());
            return newsuite;
        }
    </script>
</head>

<body>
<h1><?php echo $title; ?></h1>

<p>This page contains a suite of tests for testing JsUnit.</p>
</body>
</html>