[Home](../index.md)

OpenID Authentication System
============================

Gardens uses OpenID to maintain user logins and a centralized user system. All gardens small business platform users are required to be drupalgardens.com users. Enterprise sites differ from this. Sites might allow local logins or authenticate users with Janrain. [Read more on Janrain](janrain.md).

Short intro to OpenID
---------------------

The very basic OpenID idea is that you have an ID and can prove that you have possession/control over it. Although OpenIDs are not required to be URLs, they often are. Drupal comes with a core OpenID module that lets people register and log in to the site without providing a username and password. The username is provided by the OpenID service and the password is never stored or requested by the Drupal site.

The basic flow of OpenID is that when you log in or register, you provide your OpenID URL, such as http://me.yahoo.com/gabor.hojtsy. The core OpenID module requests that URL and looks for indicators of the OpenID endpoint URL in the HTTP headers and in HTML meta tags. Once that is found, the endpoint is contacted to set up an association (share keys with the OpenID client), so the communication can be signed proper. Then in the third HTTP request, the actual OpenID transaction begins, and the client basically asks whether the server can prove the user is in possession of that URL. The server then authenticates the user (if not already logged in), and usually asks if the user trusts the client to some of the private data that is going to be shared (such as the email address of the user). If the user does trust the client, the data is sent back signed, the client verifies it and the user is logged in or registered with the provided OpenID URL saved as belonging to the user on registration, so on next login we know that its an existing user to log in. Password are only ever provided to the server.

Application of OpenID in Gardens
--------------------------------

Because Drupal already has an OpenID client module, and an OpenID Provider contributed module for the server side is available, we used these components to set up our user database. However, if we'd use them out of the box, (a) users would need to know their gardens OpenID URL (b) users would need to verify each gardens site individually for authorization of personal data (c) we could not log people out of both the gardens site and gardener at the same time.

So we built one layer of abstraction each on both the gardens and gardener side to simplify the process. We hide the management of OpenIDs on the client and server side, and automate the trust verification process. All-in-all we use the data structures and most of the protocol (methods, key setup, communication signing, etc) of OpenID but not the user interface that is provided by Drupal core or the OpenID Provider module. Each user has an OpenID in the form of https://www.drupalgardens.com/user/$uid/identity where $uid is the only variable part.

Initiation of the login process
-------------------------------

There are three ways that the login process can be initiated:

  1. The simplest is the login links on the mysites page, which log people in immediately to their sites. This uses a custom callback on the gardens site which is located at /gardens-login and takes the OpenID as a GET argument. This initiates a regular OpenID login flow (since the OpenID URL is known), and logs the user in via the gardener (see below for details).
  2. A bit more complex is the "Register" and "Login" links on gardens sites (at /gardener/login). We don't know the OpenID of the user there, so we set up an "Identifier select" requests for the gardener, which basically means we know the server we want the user to be authenticated with but don't know the exact OpenID on that server. The gardener needs to look up the OpenID for the user (if/once logged in), and send it back.
  3. Finally, if you request a new password via email, or need to validate your email address (at /gardener/reset and/gardener/validate), the login link you get points to the gardens site with a page redirecting to the gardener to facilitate the process. This flow then redirects to the (1) method once the user password is changed and the login can be done on the gardens site with the now known OpenID.

All (1), (2) and (3) can happen both in and outside the overlay, although 1 usually happens outside and the user gets to the site right away.

Handling of the login process on the gardener
---------------------------------------------

Once the login process reaches gardener in either (1), (2) or at close to the end of (3), the gardener responds to the login request at /openid/provider. Because we only provide OpenID service for gardens sites, most of the wrapper code there deals with verifying the request came from a gardens site. It looks up the domain, attempts to find custom domains, and even does IP address lookups if the custom domain was just partly registered with us. For example, most people think www. and www-less domains are the same, and they only register one with us but want both to work.

If the site is not found (unknown source site used for login), that is usually due to domain mismatch. Such as that the domain was removed from the service but some DNS providers still point to our IPs. [Exported sites](export.md) that did not remove their domain from our service but do not host with us anymore and are in the process of DNS changes are yet another example.

If the domain is identified as trusted, the login process can start. If the user is not yet logged in, they'll get the login form right away. If they are logged in, and they are not logging into a site owned by them, or their login on gardener was over 5 minutes ago, we present them a verification form, that lets them log out and log in as someone else, or verify they are still the same person and want to log in. If the site is owned by the user of the user was logged in to gardener just a couple minutes ago, we log her in directly (as in we send back our OpenID response to the client).
Once we know the gardens site and the user ID on the gardener, we also save the login data to the OpenID provider tables. This marks the site as trusted for the user and is also used to display the "you also have accounts at these sites" table on the mysites page. So we know the last login dates for every user on every site centrally.

Final parts of the login process on the gardens site
----------------------------------------------------

Once we get back to the gardens site, the key callback path there is /openid/authenticate which handles the data coming back from the OpenID service. This uses openid_complete() to process and verify the response and either registers a new user account or logs the user in if the OpenID was already associated with a user.

Attribute exchange
------------------

OpenID has attribute exchange support which in short means that we can request and send over additional user data in the request/response. Drupal core and the OpenID Provider module already supports username and email address exchange, and we extended both to support user picture exchange. The user picture URL and last change timestamp is requested and transmitted. If the user picture changed since the last update on the client, the new one is downloaded, saved and associated with the user.

Logging users out
-----------------

Logouts have nothing to do with OpenID but it is logical to discuss here, because it completes the circle with login. We override the logout callback to not only log out on the client side but also redirect to the gardener first to log the user out there too. This is implemented at gardens-logout/% on the gardener. This logs the user out of the gardener as well. None of the other gardens site that the user was logged in will she be logged out of.

When things go wrong
--------------------

We've seen some typical issues with OpenID logins, here are the most frequent:

  - Unknown source site: possible causes explained above; this is logged on the gardener and stops the login flow
  - OpenID session information missing; logged on the gardens site, means that the OpenID data the client sent was not saved properly in the session and cannot be compared to the server response for validation; this kills the login process

Unfortunate side effects of this design
---------------------------------------

  - Attribute exchange only happens on logins. If you change a username or email address or user picture on the gardener, you still need to log into every site where you have an account for it to propagate.
  - Logout can only happen on the client and the server, not the other clients, which have no information about the logout process.
  - The login, registration, new password setup, etc. process cannot be localized at all because it happens outside of the gardens site. It will happen in English regardless of the language of the website in question.

Setting up local sites for OpenID
---------------------------------

See [local setup](../engineer/local_setup.md) for general information on local setup, these are OpenID specific pieces.

TL;DR: install your gardens site so it knows about your Gardener URL and your site maintainer account OpenID on the gardener. Create a mock site node on the gardener with the local URL of your Gardens site, so it recognizes the site as a known site managed by the Gardener.

First you'll need a local Gardener setup. There are no special requirements for this gardener, but take note of the URL of the site. For this example, we assume it is http://gardener.local/.

When setting up your local Gardens site, the site configuration screen has the following significant fields for OpenID (all URLs derived from your local Gardener URL):

 - *Site maintenance account Gardens OpenID*. We don’t have this user on the gardener in this setup, so just ensure that the format of the OpenID is right. For example, make up user 100: http://gardener.local/user/100/identity
 - *Gardens site owner account OpenID*: This is important! Use the user ID you have on the Gardener you just installed above: http://gardener.local/user/1/identity (if you have user #1 on your local Gardener). Other details of the user are not relevant and will be updated on OpenID login.
 - *Drupal Gardener URL*: http://gardener.local (no slash at the end).

The rest of the fields are not significant for OpenID setup.

At this point, if you complete the site setup, log out and try to log back in or register, you’ll get a "Your login could not be completed successfully, due to an internal error. Try again later." error. This is basically due to the Gardener not knowing about your site yet. Gardener is not working as an open to the world OpenID service, so if your site is not recognized as managed by the Gardener, it will not let you log into it. Let’s make that connection and all will be working fine.

  1. Go back to your local Gardener site (at http://gardener.local/) and log in as the initial admin user you’ve created. If you are redirected to your gardens site, that is a side effect of our OpenID session handling. Go back to gardener, and you’ll be logged in.
  2. Go to /node/add/site on the site and create your site node.
    - Fill in all required fields.
    - The internal looking Acquia Hosting site identifier and DB Cluster ID fields can be arbitrary numbers for this scenario.
    - You can ignore almost all optional fields except the following:
      - Set the full URL to the full URL of your local gardens site setup, in this example it is http://gardens.local
      - Set the domain name to the domain name of your local gardens setup, in this example: gardens.local
    - Set status to completed to avoid any other process kicking in to deal with your site.
  3. Log out of the gardener.

Now if you go back to your gardens site, you'll be able to log in using your *Gardener* credentials. You should also be able to create new user accounts or ask for new passwords and check the OpenID login flow with new password requests and registration.
