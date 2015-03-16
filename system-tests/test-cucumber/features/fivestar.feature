Feature: Fivestar

  Background:
    Given a fresh gardens installation
    And the module fivestar is enabled
    And anonymous voting is enabled
    And I am logged in as our testuser


  @poltergeist @utest
  Scenario: As a user, I want fivestar to have correct initial and editing vote ratings
    Given 7 users and an anonymous one on my site
    And I create a content type named "fivestar-01" with description "fivestar type"
    And I add a field named "fivestar_derp" of type "Fivestar Rating" to "fivestar-01"
    And I create a fivestar-01 with the title "Fivestar test" and the description "herp derp"
    When I vote twice on "Fivestar test"
    Then I should see correct fivestar voting results

  @utest
  Scenario: As a user, I can add voting to an existing content type
    When I have a site with an existing content type
    And I have content of that type
    And I add a fivestar field to that type
    Then My users can vote on the existing content type


  @poltergeist @utest
  Scenario Outline: As a user, I want to be able to set a custom star count
    Given I create a content type named "fivestar-02" with description "fivestar type"
    And I add a field named "fivestar_derp" of type "Fivestar Rating" to "fivestar-02"
    And I change the star count of field "fivestar_derp" for the content type "fivestar-02" to <stars>
    And I create a fivestar-02 with the title "Fivestar star count" and the description "herp derp"
    When I visit the page of content "Fivestar star count"
    Then I should see <stars> stars
  Examples:
    | stars |
    | 3     |
    | 4     |
    | 7     |
    | 8     |

  @poltergeist @utest
  Scenario Outline: As a user, I want to be able to use different fivestar widget types
    Given 7 users with role "Administrator" on my site
    And I create a content type named "fivestar-03" with description "fivestar type"
    And I add a field named "fivestar_derp" of type "Fivestar Rating" with widget "<widget>" to "fivestar-03"
    And I create a fivestar-03 with the title "Fivestar widget" and the description "herp derp"
    When I vote on "Fivestar widget" with "<widget>"
    Then I should see correct fivestar voting results for widget "<widget>"
  Examples:
    | widget                            |
    | Stars (rated while viewing)       |
    | Stars (rated while editing)       |
    | Select list (rated while editing) |

  @poltergeist @utest
  Scenario Outline: As a user, I want to be able to use different fivestar display types
    Given I create a content type named "fivestar-04" with description "fivestar type"
    And I add a field named "fivestar_derp" of type "Fivestar Rating" to "fivestar-04"
    And I change the display type of field "fivestar_derp" for content type "fivestar-04" to "<display_type>"
    And I create a fivestar-04 with the title "Fivestar display type" and the description "herp derp"
    When I visit the page of content "Fivestar display type"
    Then I should see a "<display_type>" fivestar display type
  Examples:
    | display_type |
    | As Stars     |
    | Rating       |
    | Percentage   |

  @poltergeist @utest
  Scenario Outline: As a user, I want to be able to set a seperate display type for teasers
    Given I create a content type named "fivestar-05" with description "fivestar type"
    And I add a field named "fivestar_derp" of type "Fivestar Rating" to "fivestar-05"
    And I change the display type of field "fivestar_derp" for content type "fivestar-05" to "<normal_type>"
    And I change the teaser display type of field "fivestar_derp" for content type "fivestar-05" to "<teaser_type>"
    And I create a fivestar-05 with the title "Fivestar teaser type" and the description "herp derp"
    When I visit "the homepage"
    Then I should see a "<teaser_type>" fivestar display type
    When I visit the page of content "Fivestar teaser type"
    Then I should see a "<normal_type>" fivestar display type
  Examples:
    | teaser_type | normal_type |
    | Rating      | As Stars    |
    | As Stars    | Percentage  |
