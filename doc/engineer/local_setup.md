[Home](../index.md)

Local environment setup
=======================
How to set up your local environment for development and to access the
production system.

Git repositories
----------------
* https://github.com/acquia/aq

    A set of useful drush command to get info from the production systems.
    Useful for all engineers.

* https://github.com/acquia/gardens

    The Gardens site codebase, helper tools, drush commands, etc.

* https://github.com/acquia/gardener

    The Gardener site codebase.

* https://github.com/acquia/fields

    The Master site codebase.

* https://github.com/acquia/gittools

    Essential repository for developers to handle git hooks, conflicts.

You do not have to fork these repositories, you can simply clone them directly
onto your machine. Once you have your gardens, gardener and gittools codebases
make sure the one time setup is done for them.

1. There are some scripts that will need to know where the gittools is, so add
to your ~/.bashrc or ~/.zshrc file:

        export GARDENS_GITTOOLS=/PATH/TO/GITTOOLS

1. Copy the settings found in the PATH/TO/GITTOOLS/configs/gitconfig_base to
your ~/.gitconfig.
1. Identify yourself either globally or individually for each github
repositories.

        git config --global user.name FIRSTNAME.LASTNAME
        git config --global user.email FIRSTNAME.LASTNAME@acquia.com

1. Set up rerere and the git hooks. We use git's rerere tool (REusing REcorded
REsolutions) to handle merge conflicts in the repositories. However, the cache
of previously recorded resolutions is not something that gets shared between
clones of a repository and we need to share resolutions between all developers
on the project. To do this, we keep them in our gittools repo. In order to keep
the rerere cache in the gittools repo up-to-date, we use a client-side git hook.
However, client-side git hooks are another example of a part of your git repo
that doesn't get transferred between clones, so it's a symlink to the rescue
again.

    1. Set up the Gardens repository.

            cd PATH/TO/GARDENS
            rm -R .git/rr-cache .git/hooks
            ln -s /PATH/TO/GITTOOLS/rerere/gardens .git/rr-cache
            ln -s /PATH/TO/GITTOOLS/hooks/gardens .git/hooks

    1. Set up the Gardener repository.

            cd PATH/TO/GARDENER
            rm -R .git/rr-cache .git/hooks
            ln -s /PATH/TO/GITTOOLS/rerere/gardener .git/rr-cache
            ln -s /PATH/TO/GITTOOLS/hooks/gardener .git/hooks

Necessary apps
--------------
For Mac and Windows it's possible to download and use the Dev Desktop from
[Acquia](http://www.acquia.com/downloads) to get the AMP stack installed. Linux
users will have to install the apps themselves though. It is worth noting as
well that the Dev Desktop might be using different versions, which is usually
not an issue unless some obscure bug comes up.

* Apache
    * Version: 2.2
    * Example:

            ./configure --prefix=/PATH/TO/DIR --enable-so --enable-rewrite --enable-ssl

    * Set up the directory for the Gardens and Gardener sites in the httpd.conf.

      Replace /PATH/TO/GARDENS and /PATH/TO/GARDENER.

            <Directory "/PATH/TO/GARDENS/gardens/docroot">
                Options Indexes FollowSymLinks
                AllowOverride all
                Order Deny,Allow
                Deny from all
                Allow from 127.0.0.1
                RewriteEngine on
                RewriteBase /
                RewriteCond %{REQUEST_FILENAME} !-f
                RewriteCond %{REQUEST_FILENAME} !-d
                RewriteRule ^(.*)$ index.php?q=$1 [L,QSA]
            </Directory>
            <Directory "/PATH/TO/GARDENER/gardener/docroot">
                Options Indexes FollowSymLinks
                AllowOverride all
                Order Deny,Allow
                Deny from all
                Allow from 127.0.0.1
            </Directory>

    * Set up a virtual host for the Gardens and Gardener sites.

      Replace /PATH/TO/GARDENS and /PATH/TO/GARDENER.

            <VirtualHost *:80>
                ServerAdmin webmaster@dummy-host.example.com
                DocumentRoot "/PATH/TO/GARDENS/gardens/docroot"
                ServerName gardens.dev
                ErrorLog "gardens.dev-error_log"
                CustomLog "logs/gardens.dev-access_log" common
            </VirtualHost>
            <VirtualHost *:80>
                ServerAdmin webmaster@dummy-host.example.com
                DocumentRoot "/PATH/TO/GARDENER/gardener/docroot"
                ServerName gardener.dev
                ErrorLog "gardener.dev-error_log"
                CustomLog "logs/gardener.dev-access_log" common
            </VirtualHost>

    * is this all that's needed ?
* PHP
    * Production is running on PHP 5.2. Running 5.3 could be possible but the
    Gardener which is on Drupal 6 might have some issues with it.
    * Todo: OpenSSL extension.
    * Example:

            ./configure \
              --with-apxs2=/PATH/TO/APACHE/bin/apxs \
              --with-mysql \
              --with-curl \
              --with-mcrypt \
              --with-pdo-mysql \
              --enable-shared \
              --with-zlib \
              --enable-mbstring \
              --with-gmp \
              --enable-sysvsem \
              --with-gd \
              --with-jpeg-dir \
              --enable-soap \
              --enable-bcmath


    * is this all that's needed ?

* Percona 5.5
* Ruby 1.8
* Drush 4

Fresh site installation
-----------------------
* Gardener

    * You can use Drupal's install.php, just select the Gardener profile.
    * With drush first set up the db_url in the settings.php and then
      ```drush --yes site-install gardener```

    * In the repository's root there is a misc directory with a setup script.

            php setup.php \
              --db_user=$dbuser \
              --db_pass=$dbpass \
              --db_path=$database  \
              --db_host=$dbhost \
              --site_name=$sitename \
              --username=$admin_username \
              --password=$admin_passowrd \
              --email=$admin_email \
              --base_url=$gardener_base_url


* Gardens
    1. Create a database for your site (for example gardens_trunk).
    1. While you could install using Drupal's install.php it might be easier to 
    use the install script we have in the repository.
      * The script will install a Drupal site, and set up the site directory
      for you. When calling the install_gardens.php make sure to use a path
      even when you are in the right directory because otherwise due to a bug
      it will not detect properly where to put the settings.php.
      ```php /path/to/directory/install_gardens.php database="my_local_db" username="my_local_db_user" password="my_local_db_pass" site_name="My Gardens Site" name="Site Administrator" pass="my_site_password" url="http://gardens.dev/install.php"```
      * Currently we have one gardens profile that is being used by most of
      the clients (SMB as in small and medium business). Bigger enterprises
      on the other hand have their own install profile but even for them the
      installation starts with the gardens profile and at a later step the
      process switches over to the proper profile. The install script can do
      this for you as well, just add an extra parameter ```gardens_client_name="PROFILE_NAME"```
    1. Make sure there's an entry in the /etc/hosts file for the site.
    ```127.0.1.1       gardens.dev```
    1. The Gardens sites use Themebuilder which will be writing to the disk. To
    allow errors due to missing write permissions either chmod the site's
    directory or chown to the apache's user.
    ```chown -R daemon:daemon docroot/sites/gardens.dev```
    1. Some extra configurational lines for the settings.php might help
    development, like avoiding caching.

                $conf['acquia_gardens_gardener_url'] = 'http://gardener';
                $conf['preprocess_css'] = FALSE;
                $conf['preprocess_js'] = FALSE;
                $conf['acquia_gardens_keep_js_css_caching_off'] = TRUE;
                $conf['gardens_client_site_verification_status'] = array('verified' => TRUE);
                $conf['gardens_client_send_stats'] = FALSE;
                $conf['gardens_client_phone_home'] = FALSE;
                $conf['cache'] = 0;
                $conf['page_cache_maximum_age'] = 300;
                // We can't use an external cache if we are trying to invoke these hooks.
                $conf['page_cache_invoke_hooks'] = FALSE;
                if (!class_exists('DrupalFakeCache')) {
                  $conf['cache_backends'][] = 'includes/cache-install.inc';
                }
                $conf['cache_class_cache_page'] = 'DrupalFakeCache';
                $conf['cache_default_class'] = 'DrupalFakeCache';
                $conf['error_level'] = 2; // ERROR_REPORTING_DISPLAY_ALL
                $conf['oembedembedly_api_key'] = 'f9b33512d44f11e0a19b4040d3dc5c07';
                $conf['acquia_gardens_developer_mode'] = TRUE;
                $conf['acquia_gardens_local_user_accounts'] = TRUE;
                $conf['site_template'] = 'blog';
                $_ENV['AH_SITE_ENVIRONMENT'] = 'development';
    1. Setting up xmlrpc to the gardener requires creating a file at /mnt/gfs/nobackup/gardens_xmlrpc_creds.ini that uses INI format:
```
[gardener]
hostname = "http://gardener.localhost"
username = "acquiagardensrpc"
password = "[password]"
```

Helpers
-------
* Add a file to /usr/ah/lib/acquia-fields-connect.php to make the local sites
work.

        <?php
        function netrc_read() {}

* By default when you want to log into a site you have to log into Bastion and
only then you will be able to go to the desired site and then repeat this
process every time you disconnect from it or want to do it again from an other
console. To avoid this repetition we have an SSH config that allows us to go
through an existing SSH Bastion connection so we just log in once and then all
other connections will reuse it. You will need a directory where some temporary
files can be stored for this feature (like ~/.ssh/tmp).

  ~/.ssh/config

  Replace BASTION\_USERNAME and /PATH.

        ControlMaster auto
        ControlPath /PATH/tmp/%h_%p_%r

        Host bastion
          HostName bastion-21.network.hosting.acquia.com
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

        Host *.*.hosting.acquia.com
          ProxyCommand ssh bastion nc %h %p
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

        Host *.gardens.f.e2a.us
          ProxyCommand ssh bastion nc %h %p
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

        Host *.wmg-egardens.f.e2a.us
          ProxyCommand ssh bastion nc %h %p
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

        Host *.fpmg-egardens.f.e2a.us
          ProxyCommand ssh bastion nc %h %p
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

        Host *.enterprise-g1.f.e2a.us
          ProxyCommand ssh bastion nc %h %p
          User BASTION_USERNAME
          Port 40506
          ServerAliveInterval 60
          ForwardAgent yes

* We are using drush a lot. Drush can also be told what site to bootstrap into.
To allow typing a lot we are doing this with an alias. The following example
would allow to run a `drush @g sqlc` to jump into the local gardens site
database regardless of what directory you are in while `drush @gr sqlc` would
jump into the local gardener's database.

  ~/.drush/g.aliases.drushrc.php

  Replace /PATH/TO/GARDENS and /PATH/TO/GARDENER

        <?php
        $aliases['g'] = array(
          'root' => '/PATH/TO/GARDENS/gardens/docroot',
          'uri' => 'http://gardens.dev',
          'include' => '/PATH/TO/GARDENS/gardens/hosting-drush/',
        );
        $aliases['gr'] = array(
          'root' => '/PATH/TO/GARDENER/gardener/docroot',
          'uri' => 'http://gardener.dev',
          'include' => '/PATH/TO/GARDENER/gardener/hosting-drush/',
        );

* There are some drush commands that the engineers use regularly. They also have
lots of arguments which do not change so we have set up some aliases to help
with the usage. A bit later there will be examples for the usage. The
DB\_USERNAME and the DB\_PASSWORD are the credentials to your local database.

  ~/.bash_aliases

  Replace /PATH/TO/AQ, BASTION\_USERNAME, /PATH/TO/GARDENS, DB\_USERNAME,
  DB\_PASSWORD.

        alias gsinfo='drush -i /PATH/TO/AQ/aq/ gs-site-info --user=BASTION_USERNAME'
        alias getsite='drush @g -i /PATH/TO/GARDENS/gardens/hosting-drush/ getsite --db-username="DB_USERNAME" --db-password="DB_PASSWORD" --remote-username=BASTION_USERNAME'
        alias clear_varnish='curl -X PURGE -H "X-Acquia-Purge: SITENAME" -H "Accept-Encoding: gzip" '

* When there is an issue with a site, one might want to jump to the server to
check out the settings / content / etc. There are lots of different kind of
servers though and their hostname are not always pretty so to make jumping to
servers one can have a helper bash script. Example `jump.sh smb managed 47`
would jump to managed-47 server on the SMB servers. The number can be taken by
using the gsinfo alias above `gsinfo --nid=906121 gardens` (can also use
--name=SITENAME) would fetch info about the nid 906121 site and contain a list
of servers which belongs to the site's tangle. An other way would be checking on
the Gardener which tangle the site belongs to and then checking on the master
which servers belong to the given tangle.

  /usr/local/bin/jump.sh

  Replace BASTION_USERNAME.

        #!/bin/bash

        if [[ "$1" == "smb" ]]; then
          HOST="gardens.hosting.acquia.com"
        elif [[ "$1" == "wmg" ]]; then
          HOST="wmg-egardens.hosting.acquia.com"
        elif [[ $1 == "fpmg" ]]; then
          HOST="fpmg-egardens.hosting.acquia.com"
        elif [[ $1 == "pfi" ]]; then
          HOST="enterprise-g1.hosting.acquia.com"
        elif [[ $1 == "utest" ]]; then
          HOST="utest.hosting.acquia.com"
        elif [[ $1 == "gsteamer" ]]; then
          HOST="gsteamer.hosting.acquia.com"
        else
          echo "Stage not found\n"
          exit 1
        fi

        ssh -A BASTION_USERNAME@$2-$3.$HOST -p 40506

Getting a production site
-------------------------
* Gardens

  It is possible that a bug seems to be happening solely on one site and it
  seems to be very hard to reproduce the issue locally otherwise. One might feel
  the need to debug on the live site but that is rarely a good choice. However
  there is a tool to take a copy of a site and deploy it locally and then one is
  free to add debugging lines as much as needed. The following example is using
  the bash alias "getsite" defined in the previous section:
  `getsite --stage=STAGE_NAME SITENAME TANGLE_NAME`. The STAGE\_NAME can be
  looked up in the [arch](../arch/arch.md). The SITENAME can be acquired from
  the Gardener. The TANGLE\_NAME can be found on the Gardener as well and is
  also being returned by the gsinfo script. The getsite script has a couple
  options (check in the Gardens code base - hosting-drush/gardens.drush.inc),
  can take the whole site, the database, all the files, everything, but in some
  cases there are so many files that it would take very long, so it may make
  sense to add the --theme-files-only parameter to the getsite as well, or even
  to the alias. The script will create the database, the sites directory, but
  will not set the virtual host, the /etc/hosts entries or the site directory's
  permissions, and adding some of the entries from the the dev settings.php
  might also help to  get the site work quicker.

* Gardener

  TODO
  some info on the [intranet](https://i.acquia.com/wiki/setting-gardens-and-gardener-sites-locally)
  which might be outdated though.
