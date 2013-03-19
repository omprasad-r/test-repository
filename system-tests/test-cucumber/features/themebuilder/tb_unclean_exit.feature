Feature: Theme Builder exit

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @selenium
  Scenario Outline: As user, I can re-open the theme builder and see the active draft message
    Given I am on "the homepage"
    And I open the theme builder
    And I enable theme builder power theming
    And I switch to the theme builder tab "Styles"
    And I switch to the vertical theme builder tab "Font"
    And I select element "<element>"
    And I change the font family to "Palatino"
    And I change the font size to "14"
    When I have a new session
    And I visit the homepage
    And I am logged in as our testuser
    And I press element "div#toolbar a#toolbar-link-admin-appearance"
    Then I should see the active draft message
    Examples:
      | element         |
      | site name link  |
      | site slogan     |

  @selenium
  Scenario Outline: As user, I can re-open the theme builder and see that the changed attributes are set correctly
    Given I am on "the homepage"
    And I open the theme builder
    And I enable theme builder power theming
    And I switch to the theme builder tab "Styles"
    And I switch to the vertical theme builder tab "Font"
    And I select element "<element>"
    And I change the font family to "Palatino"
    And I remember the font family of "<element>" as "expected_font_family"
    And I change the font size to "14"
    And I remember the font size of "<element>" as "expected_font_size"
    When I have a new session
    And I visit the homepage
    And I am logged in as our testuser
    And I open the theme builder
    And I switch to the theme builder tab "Styles"
    And I switch to the vertical theme builder tab "Font"
    And I select element "<element>"
    And I remember the font family of "<element>" as "actual_font_family"
    And I remember the font size of "<element>" as "actual_font_size"
    Then I should see that "<element>" equals "style-font-family" font family
    And I should see that "<element>" equals "style-font-size" font size
    And I should see that remembered value "actual_font_family" equals "expected_font_family"
    And I should see that remembered value "actual_font_size" equals "expected_font_size"
    Examples:
      | element         |
      | site name link  |
      | site slogan     |

  @selenium
  Scenario Outline: As user, I can open a second theme builder session and see the active draft message
    Given I am on "the homepage"
    And I open the theme builder
    And I enable theme builder power theming
    And I switch to the theme builder tab "Styles"
    And I switch to the vertical theme builder tab "Font"
    And I select element "<element>"
    And I change the font family to "Palatino"
    And I change the font size to "14"
    When I am in other_user's browser
    And I visit the homepage
    And I am logged in as our testuser
    And I press element "div#toolbar a#toolbar-link-admin-appearance"
    Then I should see the active draft message
    Examples:
      | element         |
      | site name link  |
      | site slogan     |


