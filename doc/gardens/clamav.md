[Home](../index.md)

ClamAV Overview
===============
We are using clamav to scan file uploads in gardens. In order to reduce memory usage on the web servers, we have opted to use a separate clamd service running on a custom server. For history, the set up tickets are in Jira: OP-6597 and DG-3838. The server is a custom build from ops using an elastic IP so that we can plan for HA. 

Currently, the deployed clamd server is custom-274.gardens.hosting.acquia.com and the Elastic IP is 23.23.126.74.

The custom-274 server is running clamav-daemon on port 3310. This port is locked down via EC2 security groups so that it only accepts traffic from other gardens stages.

Jira Tickets
------------
DG-3838
