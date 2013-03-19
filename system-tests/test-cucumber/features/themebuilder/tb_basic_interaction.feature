Feature: Theme Builder basic interaction

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @smoke
  Scenario: As user, I can do a basic modification followed by a save as
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Layout"
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I select "Palatino" from select box "style-font-family"
    And I type "14" into "style-font-size"
    And I save timestamped theme with prefix "test"

  @smoke
  @selenium
  Scenario: As user, I can use the font size slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I move the "style-font-size" slider to 30 percent
    Then I should see that "site name link" equals "style-font-size" font size

  @smoke
  Scenario: As user, I can use the color palette picker
    Given I change the window size to 1600x1200
    And I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I pick item "b" from color picker "style-font-color"
    Then I should see that "site name link" equals "style-font-color" color

  @smoke
  Scenario Outline: As user, I can set spacing margins, padding and borders by typing
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Spacing"
    And I select element "<element>"
    And I type "14" into "tb-style-<spacingtype>-top<suffix>"
    Then I should see that "<element>" equals "tb-style-<spacingtype>-top<suffix>" <spacingtype> top<suffix>
    And I type "24" into "tb-style-<spacingtype>-left<suffix>"
    Then I should see that "<element>" equals "tb-style-<spacingtype>-left<suffix>" <spacingtype> left<suffix>
    And I type "34" into "tb-style-<spacingtype>-right<suffix>"
    Then I should see that "<element>" equals "tb-style-<spacingtype>-right<suffix>" <spacingtype> right<suffix>
    And I type "4" into "tb-style-<spacingtype>-bottom<suffix>"
    Then I should see that "<element>" equals "tb-style-<spacingtype>-bottom<suffix>" <spacingtype> bottom<suffix>
    Examples:
      | element         | spacingtype |   suffix    |
      | site name link  | margin      |             |
      | rotating banner | padding     |             |
      | site name link  | border      |   -width    |

  @smoke
  @selenium
  Scenario Outline: As user, I can set margins, padding and borders using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Spacing"
    And I select element "<element>"
    And I move the "tb-style-<spacingtype>-top<suffix>" slider to 30 percent
    Then I should see that "<element>" equals "tb-style-<spacingtype>-top<suffix>" <spacingtype> top<suffix>
    And I move the "tb-style-<spacingtype>-left<suffix>" slider to 50 percent
    Then I should see that "<element>" equals "tb-style-<spacingtype>-left<suffix>" <spacingtype> left<suffix>
    And I move the "tb-style-<spacingtype>-right<suffix>" slider to 5 percent
    Then I should see that "<element>" equals "tb-style-<spacingtype>-right<suffix>" <spacingtype> right<suffix>
    And I move the "tb-style-<spacingtype>-bottom<suffix>" slider to 95 percent
    Then I should see that "<element>" equals "tb-style-<spacingtype>-bottom<suffix>" <spacingtype> bottom<suffix>
    Examples:
      | element         | spacingtype |   suffix    |
      | rotating banner | margin      |             |
      | site name link  | padding     |             |
      | site name link  | border      |   -width    |

  @selenium
  Scenario Outline: As a user, I should be able to switch between the vertical tabs under the "Advanced tab"
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "<vertical_tab>"
    Then I should see that "<vertical_tab>" is the currently active vertical tab
    Examples:
      | vertical_tab        |
      |   Styles CSS        |
      |   Custom CSS        |
      |   Viewport settings |

  Scenario: As a user, I can publish themes several times in a row without breaking the installation
    Given I am on "the homepage"
    And I open the theme builder
    And I select layout abc for all pages
    And I select layout acb for all pages
    And I save the current theme as "theme_publish"
    And I publish multiple themes with the names:
      | theme_name      |
      | publish_one     |
      | publish_two     |
      | publish_three   |
    Then I should see that the theme folder contains theme "theme_publish"
