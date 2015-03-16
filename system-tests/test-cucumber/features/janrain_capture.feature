Feature: Janrain Capture

  Background:
    Given a fresh gardens installation
    And the module janrain_capture is enabled
    #And janrain_capture has been configured with a valid API key and secret
    #And the Janrain Capture block has been enabled in a region of the site

  @wmg @utest
  Scenario: As an anonymous user, I want to be able to log in to the site using Janrain Capture
    Given I click on the Login / Register link provided by the Janrain Capture block
    Then I should see a modal dialog with a choice to either sign in my an account from Google, Yahoo, AOL or OpenID, or to sign in / create an account using an email address and password.

  @wmg @utest
  Scenario: As an authenticated user, I want to be able to edit my profile using Janrain Capture
    Given I have logged into the site using the Janrain Capture login block
    And I click on the "View/Edit profile" link in the Janrain Capture block
    Then I should see a an overlay where I can edit my account details and hit save or cancel to go back to the site.

  @wmg @utest @wip
  Scenario: As an authenticated user, my user account should have a Janrain Capture GUID value
    Given janrain_capture has been configured with a GUID-enabled API key and secret
    And warner_capture module is enabled
    And server_variables module is enabled
    And I have logged into the site using the Janrain Capture login block
    Then I should find a value in any page Javascript object window.Drupal.settings.server_variables.capture_guid matching the pattern "{0-9a-f}8-{0-9a-f}4-{0-9a-f}4-{0-9a-f}4-{0-9a-f}12"

  @wmg @utest @wip
  Scenario: As an authenticated user, my Janrain Capture GUID value should be available to Bunchball Javascript
    Given janrain_capture has been configured with a GUID-enabled API key and secret
    And bunchball module is enabled and configured with a valid sandbox API key and secret
    And bunchball module is configured to use "Janrain GUID" as user identifier
    Then I should find a value in any page Javascript object window.Drupal.settings.bunchballNitroConnection.connectionParams.userId containing my Janrain Capture GUID

  @wmg @utest @wip
  Scenario: As an authenticated user, I should not be able to edit the value of the Janrain Capture GUID field
    Given janrain_capture has been configured with a GUID-enabled API key and secret
    And I have logged into the site using the Janrain Capture login block
    And I click my username link to view my user account
    And I click "Edit Profile" to edit my Drupal site local profile
    Then I should not see a form field containing "GUID" in the label

  @wmg @utest
  Scenario: As an authenticated user, I can login via different providers
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an authenticated user, I should see an error message, when the capture service is down
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an administrator, I can configure the site to use capture only without a capture account
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an administrator, I can configure the site to use capture only with a capture account
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As a user, I can log in as an existing user using capture only
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As a user, I can log in as a new user using capture only
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As a user, I can log to a site via the gardener using capture
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an administrator, I can unset capture only login
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an authenticated user, I can edit user profile settings
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As an administrator, I can configure the capture field mapping
    Given this hasn't been automated yet

  @wmg @utest
  Scenario: As a user, I should be redirected to the correct page after being prompted to log in
    Given this hasn't been automated yet
