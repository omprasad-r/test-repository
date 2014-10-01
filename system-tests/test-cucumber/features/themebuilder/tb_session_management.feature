Feature: Theme builder session management

  Background:
    Given a fresh gardens installation
    Given I am logged in as our testuser

  @non-utest
  Scenario: As a user, I can quickly open and close the theme builder without leaving behind session files
    Given I am on "the homepage"
    And I repeat the following steps 20 times:
      | step                                                                                                                          |
      | I open the theme builder                                                                                                      |
      | I should see the "acq_qatestuser_session" theme directory                                                                     |
      | I should see the files "template.php, custom.css, advanced.css, palette.css" in the "acq_qatestuser_session" theme directory  |
      | I close the theme builder                                                                                                     |
      | I should not see the "acq_qatestuser_session" theme directory                                                                 |
