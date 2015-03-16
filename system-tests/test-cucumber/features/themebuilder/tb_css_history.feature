Feature: Theme Builder CSS history

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @utest @selenium
  Scenario Outline: As a user, I can set CSS font styles in the theme builder and can see them in the history
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can set selectively hide CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I remember the font size of "<desired_element>" as "original_font_size"
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    When I hide the CSS rule for element "<desired_element>"
    And I remember the font size of "<desired_element>" as "hidden_font_size"
    Then I should see that remembered value "original_font_size" not equals "modified_font_size"
    And I should see that remembered value "original_font_size" equals "hidden_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can set selectively hide and re-enable CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    When I hide the CSS rule for element "<desired_element>"
    And I show the CSS rule for element "<desired_element>"
    And I remember the font size of "<desired_element>" as "enabled_font_size"
    Then I should see that remembered value "modified_font_size" equals "enabled_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can delete CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I remember the font size of "<desired_element>" as "original_font_size"
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    When I delete the CSS rule for element "<desired_element>"
    And I remember the font size of "<desired_element>" as "deleted_font_size"
    Then I should see that the CSS history rule for "<desired_element>" is not present
    And I should see that remembered value "original_font_size" not equals "modified_font_size"
    And I should see that remembered value "original_font_size" equals "deleted_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can hide all CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I remember the font size of "<desired_element>" as "original_font_size"
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    When I hide all CSS rules
    And I remember the font size of "<desired_element>" as "deleted_font_size"
    Then I should see that all CSS history rules are hidden
    And I should see that remembered value "original_font_size" not equals "modified_font_size"
    And I should see that remembered value "original_font_size" equals "deleted_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can hide and re-enable all CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    When I hide all CSS rules
    And I show all CSS rules
    And I remember the font size of "<desired_element>" as "enabled_font_size"
    Then I should see that all CSS history rules are not hidden
    And I should see that remembered value "modified_font_size" equals "enabled_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can save the theme and see correct CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    When I save the current theme
    And I reload the current page
    And I remember the font size of "<desired_element>" as "reloaded_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    And I should see that remembered value "modified_font_size" equals "reloaded_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can save the named theme and see correct CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    And I save the current theme as "theme_css_history"
    When I load the theme "theme_css_history"
    And I remember the font size of "<desired_element>" as "reloaded_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    And I should see that remembered value "modified_font_size" equals "reloaded_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |

  @utest @selenium
  Scenario Outline: As a user, I can publish the theme and see correct CSS rules
    Given I am on "the homepage"
    And I open the theme builder
    And I change element "<desired_element>" to color "b" and font size "0"
    And I remember the font size of "<desired_element>" as "modified_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    When I publish the current theme
    And I remember the font size of "<desired_element>" as "reloaded_font_size"
    And I switch to theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Styles CSS"
    Then I should see that the CSS history rule for "<desired_element>" is present
    And I should see that remembered value "modified_font_size" equals "reloaded_font_size"
    Examples:
      | desired_element               |
      | #header-region #site-name a   |
      | #main h2 a                    |


  @utest
  Scenario: As a user, I can append custom CSS code and observe changes in preview
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can append custom CSS code and save the theme
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can select a CSS item to style and see that the right one is highlighted
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can selectively hide and show CSS selector attributes
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can delete/undo CSS rule attributes
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can delete/undo CSS rule attributes and preview, save and publish the theme
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can change the viewport metatag
    Given this hasn't been automated yet
