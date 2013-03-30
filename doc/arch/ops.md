Ops-ish tricks for gardens
==========================
A collection of ops tricks that can be handy for gardens development - typically ops will handle things like adding webnodes to production clusters, but it is outside of their responsibility to do the same in staging environments.

Most potentially damaging infrastructure changes in production environments should be requested from ops **unless you really know what you're doing.**

Setup
-----
Any fields-provision.php commands need to be run from the bastion, so refer to [access instructions for the bastion](access.md)

As usual, on bastion use **fstart** to choose an account (gardens-dev or gardens-prod) and **fstage** to choose your stage (cluster) - eg. utest,enterprise-g1-staging.

Checking RPC access is set up
-----------------------------
After using fstart and fstage to set up some environment variables, it's worth checking that everything's in the right place and you have access to run commands.

`fields-provision.php --site-info-base tangle001` 

should give you some basic information about tangle001.  If you get HTTP access errors, you might need to get a new rpc password.  It seems only possible to store a single rpc password for each of gardens-dev and gardens-prod at one time, because the passwords share the same machine name in the netrc file.  To check first of all that you can get a RPC password, run:

`fsshp master 'sudo ah-get-rpc-password <username>'`

If you see output including:


    machine …   
    login … 
    password … 

then you are able to get a RPC password and you can store it in your dynamic netrc file like this:

`fsshp master 'sudo ah-get-rpc-password <username>' > $NETRC.dynamic`

which will then replace anything that was previously in the file.

*You didn't hear this from me, but there are occasions where you might have root access to the master, but your username is not permitted to obtain a RPC password.  If you have root access to the master, then you can get the RPC password for literally any user, which might help you get done what you need done before getting your own user set up.  Having root access to the master implies that you are allowed to do this as root access to the master gives you permission to do almost everything*

Adding SSH keys to the tangle user
-----------------------------------
This can be useful, particularly when working with external contractors, who will not have full administrative access to sites that they might need to work on.

Store the user's public key in a local file on the bastion and use this command to add their key to the site user:


`fields-provision.php --user-setpubkey <site>:<username>:<pubkey-file-on-bastion>`

\<site> would be a site name such as "tangle001" in this case.  Username should usually be first initial + surname and maybe a hint about which partner company they work for.  You must provide the full path to the local public key file, using "~" for your home directory currently doesn't work on the bastion for setting public key files - you need to manually expand the path to be the full path.

The user will then need to SSH as the site user eg "tangle001" using the default ssh port, not the administrative port.


Setting the code branch on a staging tangle
--------------------------------------------
This is usually necessary for testing code under development on realistic staging environments. It shouldn't be neccessary or possible to do this on production unless you are the duty release engineer.

You can assume that a staging cluster is already set to the right VCS type and repository, and that all you'll need to do is change branches sometimes.  
`fields-provision.php —set-site-vcs-path tangle001:<branch-name>`

After changing branches, it's usually a good idea to check test sites for updates using `drush updb`.  It's not usually a good idea to attempt to go back to older code if DB updates ran.

Clearing caches is also a good thing to do after changing code branches - it won't happen automatically (unless you do run `drush updb`)


Configuring the cron service for sites
--------------------------------------
We don't use actual crontabs on the same web nodes as the codebase to run cron, we use a special menu callback which has some trivial protection against arbitrary external use by adding a trivial hash to check access.

There is a java-based cron service that takes care of running all crons on all sites in a way which won't overload the system.

Use the following command to configure the cron service.  Here is an example that sets the cron service to run cron on all sites every 5 minutes for just the gardens_pdf_rendition module

`./fields-provision.php --cron-add cron:*/5:*:*:*:* --cmd '/usr/bin/java -Djava.vm.pid=$$ -jar /mnt/www/html/cron/cronService.jar -d /mnt/www/html/cron  --select gardens_pdf_rendition'`

The only parameters that are likely to need tweaking are: 

 - "cron:\*/5:\*:\*:\*:\*" - cron in this case identifies the machine running the job, and the timings are in [regular crontab format](http://en.wikipedia.org/wiki/Cron).
 - --select allows you to specify specific modules as a comma-separated list to run cron only on selected modules.  Omit the parameter entirely to perform a full cron run including all modules.  It is **critical** that there is no "=" sign between the select and the modules list

Adding a web node to a tangle
-----------------------------
There's really only one reason you'd need to do this - to test whether anything about in-development code breaks when adding a new web node.

It's good to get information on the stage you're deploying in in advance - eg. region, availability zone, operating system

`fields-provision.php --server-allocate managed:1:managed --region <region> --availability-zone <availability-zone> --instance-type=<size> --os=<os>`

This should tell you the name of the allocated server which you can use in subsequent commands.  The servername in this case should turn out to be something like managed-N due to the allocation type + prefix above.

Check the master UI or on the command line to find the appropriate fs-cluster name for the tangle you're deploying.

`fields-provision.php --fs-cluster-add-servers clustername:servername1,srv2,…`

`fields-provision.php --launcher --server <server-name-from-above>`

Wait … quite a while … even if it looks like it's stuck.  Don't shut your laptop whilst launching.

`fields-provision.php --site-set-web sitename[:status] --webs <servername-from-above>`

Status can be:

 -  active: Deployed, ready, and in active rotation.
 - inactive: Deployed and ready, but NOT in active rotation.
 - deploy (default): To be deployed, and then status changed to 'active' when ready.



You can suspend the server when you're finished with it for a while: 

`fields-provision.php --suspend  managed-22`


Here's a working example from Enterprise G1 staging:

    fields-provision.php --server-allocate managed:1:managed --region eu-west-1 --availability-zone eu-west-1a --instance-type c1.medium --os hardy
    fields-provision.php --fs-cluster-add-servers fs-tangle001:managed-22
    fields-provision.php --launcher --server managed-22
    fields-provision.php --site-set-web tangle001:active --webs managed-22
    fields-provision.php --suspend  managed-22

Kicking the task server
------------------------

Sometimes, the task server responsible for managing the queue of tasks to run within the gardens cluster hangs.  Very occasionally, it might die completely.  The first sign that the task server is in trouble might be that site creation partially fails.  You might see a site creation appear to succeed, and then redirect you to the "site not found" page.  There are many other possible causes of site creation failing though, so check whether the task server is processing jobs via the master web UI to verify.

If a task server dies or fails on a production cluster, the best course of action is to alert ops and ask them to restart it, but on staging clusters, doing so isn't really their responsibility.

If the task server dies, then all it takes to restart it is locate the backup-\* server it runs on (use the hosting master web UI) and run puppet, but it's neccessary to know how to check if it died and how to kill it if it just hung.  On most production and staging clusters, there's usually only one backup server.  On gsteamer and SMB, there are 2, so assume if the task server is having trouble, then it's probably *both* of them.

SSH into the backup server, and list processes matching "task".  I tend to use:

`ps -ef | grep task`

If you see nothing but possibly the grep process, then just run:

`puppetd --test` 

to restart the task server.

If the task server is running, you should see something like:

    root     20541     1  0  2012 ?        00:00:00 daemon --name=ah-task-server --unsafe --noconfig --env=PATH=/usr/local/sbin:/usr/local/bin:/usr/local/ec2-api-tools/bin:/sbin:/bin:/usr/sbin:/usr/bin --env=HOME=/root --env=FIELDS_STAGE=gsteamer --env=EC2_ACCOUNT=gardens-dev --env=JAVA_HOME=/usr --env=EC2_HOME=/usr/local/ec2-api-tools --env=EC2_PRIVATE_KEY=/root/ec2/pk.pem --env=EC2_CERT=/root/ec2/cert.pem --env=AHBOT=true ssh-agent ah-task-server
    root     20543 20541  0  2012 ?        05:25:49 ruby /usr/local/sbin/ah-task-server
    root     20544 20543  0  2012 ?        00:01:01 ssh-agent ah-task-server

The cleanest way to kill these is from the last child first working upwards.  In this case:

    kill 20544
    kill 20543
    
You'll probably find there are no more at that point.

Then restart it by running puppet again:

`puppetd --test`

It will take a minute or 2.  After that, check the processes are running, and check the master web UI once more to verify tasks are processing.  It might be worth also checking syslog for info on what went wrong:

`less /var/log/syslog` … search for "CRIT"

If it has been down for a while, then the event might be in an older log (syslog.1.gz, syslog.2.gz etc)
