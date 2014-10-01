Feature: Homepage

  @smoke
  Scenario: A user wants to go to the homepage and actually find a working gardens installation there
    Given a fresh gardens installation
    And I am on the homepage
    Then I should see "This is your site"
    And I should see "Powered by Drupal Gardens"

  Scenario: All linked images on the default starting page should be present
    Given a fresh gardens installation
    And I am on the homepage
    Then all images should be present

  # this scenario needs to run on a gardens instance linked to a gardener
  Scenario Outline: As a user, I can log in using the login iframe and get redirected to the correct page
    Given this hasn't been implemented yet
    And I am on the homepage
    When I visit <desired_page>
    Then I should see the text "You are not authorized to access this page"
    When I am logged in as our testuser using the login iframe
    Then I should be on "<desired_page>"
    Examples:
      | desired_page  |
      | /admin        |
      | /node/add     |
