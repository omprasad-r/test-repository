[Home](../index.md)

Systems Access Overview
=======================

### Bastion
First and foremost, if you are going to access the infrastructure on production stages, 
you need to get an account on the bastion. 
a stanza like

    Host bastion
      HostName bastion-21.network.hosting.acquia.com
      User <username>
      Port 40506
      ServerAliveInterval 60
      ControlMaster auto
      ControlPath /tmp/ssh_mux_%h_%p_%r
      ForwardAgent yes

In your ~.ssh/config will allow you to 
1. forward your ssh agent
2. log in like `ssh bastion` 



