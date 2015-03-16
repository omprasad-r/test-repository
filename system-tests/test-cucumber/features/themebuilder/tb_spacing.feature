
Feature: Theme Builder spacing settings

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @utest
  Scenario Outline: As a user, I can set the edge spacing of an element using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    When I set the <spacing> top spacing to 30 percent using the slider
    Then I should see that the "header region" equals "tb-style-<spacing>-top" <spacing> top
    When I set the <spacing> bottom spacing to 30 percent using the slider
    Then I should see that the "header region" equals "tb-style-<spacing>-bottom" <spacing> bottom
    When I set the <spacing> left spacing to 30 percent using the slider
    Then I should see that the "header region" equals "tb-style-<spacing>-left" <spacing> left
    When I set the <spacing> right spacing to 30 percent using the slider
    Then I should see that the "header region" equals "tb-style-<spacing>-right" <spacing> right
    Examples:
      | spacing |
      | margin  |
      | padding |
      | border  |

  @utest
  Scenario Outline: As a user, I can set the spacing of an element using text input
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    When I set the <spacing> top spacing to 20
    Then I should see that the "header region" equals "tb-style-<spacing>-top" <spacing> top
    When I set the <spacing> bottom spacing to 30
    Then I should see that the "header region" equals "tb-style-<spacing>-bottom" <spacing> bottom
    When I set the <spacing> left spacing to 35
    Then I should see that the "header region" equals "tb-style-<spacing>-left" <spacing> left
    When I set the <spacing> right spacing to 40
    Then I should see that the "header region" equals "tb-style-<spacing>-right" <spacing> right
    Examples:
      | spacing |
      | margin  |
      | padding |
      | border  |

  @utest
  Scenario Outline: As a user, I can set the border color of an element
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    When I set the border top spacing to 20
    And I pick item "<color>" from color picker "style-border-color"
    Then I should see that the "header region" equals "style-border-color" border top color
    Examples:
      | color |
      | a     |
      | b     |
      | c     |
      | d     |
      | e     |

  @utest
  Scenario Outline: As a user, I can set the corner of an element using the slider
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    When I set the <spacing> spacing to 30 percent using the slider
    Then I should see that the "header region" equals "tb-style-<spacing>-top" <spacing> top
    And I should see that the "header region" equals "tb-style-<spacing>-bottom" <spacing> bottom
    And I should see that the "header region" equals "tb-style-<spacing>-left" <spacing> left
    And I should see that the "header region" equals "tb-style-<spacing>-right" <spacing> right
    Examples:
      | spacing |
      | margin  |
      | padding |
      | border  |

  @utest
  Scenario Outline: As a user, I can change spacing settings and save, save as and preview the theme
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    And I set the border top spacing to 20
    And I set the padding bottom spacing to 10
    And I pick item "d" from color picker "style-border-color"
    When I <action> and reload the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Borders & Spacing"
    And I select element "header region"
    Then I should see that the "header region" equals "tb-style-border-top" border top
    And I should see that the "header region" equals "tb-style-padding-bottom" padding bottom
    And I should see that the "header region" equals "style-border-color" border top color
    Examples:
      | action  |
      | save    |
      | save_as |
      | publish |

  @utest
  Scenario: As a user, I want to be able to use undo and redo in combination with changes to an elements spacing parameters
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I want to be able to enable and disable power theming
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I want to have visual and textual feedback when selecting an item to theme
    Given this hasn't been automated yet
