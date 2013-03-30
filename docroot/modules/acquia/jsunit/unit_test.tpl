<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title><?php print $jsunit->getTitle(); ?></title>
    <?php print $jsunit->getCssMarkup(); ?>
    <?php print $jsunit->getJavaScriptMarkup(); ?>

    <script type="text/javascript">
      <?php print $jsunit->getJavaScriptTests(); ?>
    </script>
</head>

<body>
<h1><?php print $jsunit->getTitle() ?></h1>
<?php print $jsunit->getHTML(); ?>
<p>This page contains tests for the JsUnit Framework. To see them, take a look at the source.</p>
</body>
</html>
