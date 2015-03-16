Feature: Theme Builder typekit integration

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a user, I can enable typekit fonts

  @utest @selenium
  Scenario Outline: As a user, I can use typekit fonts in the theme builder
    Given the typekit font management is enabled
    And I am on "the homepage"
    And I open the theme builder
    When I select the font "<desired_font>" for element "<desired_element>"
    Then I should see that element "optgroup[label $= 'Typekit fonts']" contains at least 1 element
    Examples:
      | desired_element                     | desired_font  |
      | h1#site-name                        | Corpulent Web |
      | p#site-slogan                       | Givry Web     |

  @utest @selenium
  Scenario Outline: As a user, I can select a typekit font and should see it is set correctly after preview, save, save as and publish
    Given the typekit font management is enabled
    And I am on "the homepage"
    And I open the theme builder
    And I select the font "Givry Web" for element "site name link"
    Then I should see that "site name link" equals "style-font-family" font family
    When I <desired_action> and reload the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-font-family" font family
    Examples:
      | desired_action  |
      |   preview       |
      |   save          |
      |   save_as       |
      |   publish       |

  @utest
  Scenario: As a user, I can disable the typekit module and should see that the tk fonts don't show up
    Given the typekit font management is enabled
    When the module font_management is disabled
    And I am on "the homepage"
    And I open the theme builder
    Then I should not see element "optgroup[label $= 'Typekit fonts']" within "select#style-font-family"

  @utest
  Scenario: As a user, I can disable the typekit integration in the font management and should see that the tk fonts don't show up
    Given the typekit font management is enabled
    When I disable the typekit integration
    And I am on "the homepage"
    And I open the theme builder
    Then I should not see element "optgroup[label $= 'Typekit fonts']" within "select#style-font-family"

  @utest
  Scenario: As a user, I can enable monotype fonts
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can select a monotype font and should see it is set correctly after preview, save, save as and publish
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can disable the monotype integration in the font management and should see that the MT fonts don't show up
    Given this hasn't been automated yet

