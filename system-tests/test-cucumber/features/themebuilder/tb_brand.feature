Feature: Theme Builder permission

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @utest
  Scenario: As a themer, I can select a color palette other than the default.
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Brand"
    And I switch to vertical theme builder tab "Palettes"
    And I select a palette
    Then my sites palette changes

  @utest
  Scenario Outline: As a themer on a "<customer type>",  my default favion "<is>" empty.
    Given I am on "the homepage" of a "<customer_type>" site
    And I "<can_see>" a default favicon
    And I open the theme builder
    And I switch to the theme builder tab "Brand"
    And I switch to vertical theme builder tab "Logo"
    And I can see that the favicon "<is>" empty
  Examples:
    | customer_type | can_see | is     |
    | smb           | can     | is not |
    | enterprise    | cannont | is     |

  @utest
  Scenario: As a themer, I want to select a favicon that represents my brand.
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Brand"
    And I switch to vertical theme builder tab "Logo"
    And I select a favicon
    Then I get the new favicon

  @utest
  Scenario: As a themer, I want to remove an undesirable favicon
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Brand"
    And I switch to vertical theme builder tab "Logo"
    And I select remove a favicon
    Then I no longer have a favicon

  @utest
  Scenario: As a theme, I want to add a logo to my site.
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Brand"
    And I switch to vertical theme builder tab "Logo"
    And I upload a logo
    And I enable the site logo block if prompted
    And I publish my theme
    And I close the theme builder
    Then I should have my logo displayed on my site

  @utest
  Scenario: As a theme, I want to remove a logo from my site.
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Brand"
    And I switch to vertical theme builder tab "Logo"
    And I remove the logo
    And I publish my theme
    And I close the theme builder
    Then I should no longer have my logo displayed on my site






