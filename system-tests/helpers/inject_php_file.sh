sed '/managed-51/d' ~/.ssh/known_hosts > ~/.ssh/known_hosts_no51
mv ~/.ssh/known_hosts_no51 ~/.ssh/known_hosts
sed '/managed-47/d' ~/.ssh/known_hosts > ~/.ssh/known_hosts_no47
mv ~/.ssh/known_hosts_no47 ~/.ssh/known_hosts

scp testing_themefolder_related.php root@managed-51.gsteamer.hosting.acquia.com:/mnt/www/html/tangle001/docroot/
scp testing_themefolder_related.php root@managed-47.gsteamer.hosting.acquia.com:/mnt/www/html/tangle001/docroot/
