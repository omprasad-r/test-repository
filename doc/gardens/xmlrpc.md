XMLRPC Communication
====================
We use the Pear library 'XML_RPC' to communicate from the gardens sites to the gardener. The gardener site uses http basic auth (gardener configuration is in the docroot .htaccess file), and these credentials need to be stored somewhere on the web servers that run the gardens sites. We have opted to use an ini formatted file that is manually placed on these servers to contain these credentials. Please see the documentation for _acquia_gardens_xmlrpc_creds() to learn more about the location and the format.
