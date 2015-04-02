Feature: Blocks

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And I am an administrator on the site

  @utest
  Scenario: As a user, I can access both Drupal pages and static assets when site guard is disabled
    When site guard is disabled
    And I create a node
    And I upload an image to that node
    Then I can view the node
    And I can view the image by browsing to the image URL

  @utest
  Scenario: As a user, I need to enter credentials to access both Drupal pages and static assets when site guard is enabled
    When I browse to admin/config/system/site_guard
    And I tick the checkbox to enable site guard
    And I set the username to "test" and the password to "test"
    And I save the form
    Then I am required to enter credentials to access Drupal
    And I am required to enter credentials to access the image URL
    When I enter the wrong credentials "fake" and "fake"
    Then I am requested to try again
    When I enter the correct credentials "test" and "test"
    Then I can access Drupal
    And I can access the image URL
    When I open a new incognito window in my browser
    Then I am required to enter credentials to access Drupal
    And I am required to enter credentials to access the image URL
