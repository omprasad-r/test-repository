Feature: Theme Builder permission

  Background:
    Given a fresh gardens installation
    And a new user "permissionist" with the password "foo#bar#baz"
    And the role "authenticated user" has the permission "administer themes"
    And the role "authenticated user" has the permission "access toolbar"
    And the role "authenticated user" has the permission to see all theme builder tabs
    And I am logged in as user "permissionist" with the password "foo#bar#baz"

  @smoke
  Scenario Outline: As user, I can switch tabs and see only the currently active one
    Given the role "authenticated user" has the permission "administer site configuration"
    And I am on "the homepage"
    And I open the theme builder
    When I switch to theme builder tab "<desired_tab>"
    Then I should only see the currently active tab
    Examples:
      | desired_tab |
      |   Styles    |
      |   Brand     |
      |   Layout    |
      |   Advanced  |
      |   Themes    |

  @smoke
  Scenario Outline: As user, I should have access to all theme builder tabs
    Given the role "authenticated user" has the permission "administer site configuration"
    And I am on "the homepage"
    When I open the theme builder
    Then I should see all theme builder tabs

  @smoke
  Scenario Outline: As user, my access to theme builder tabs should be limited by permissions
    Given the role "authenticated user" does not have the permission "administer site configuration"
    And the role "authenticated user" does not have the permission "access themebuilder <desired_tab> tab"
    When I open the theme builder
    And I am on "the homepage"
    Then I should not see the theme builder tab "<desired_tab>"
    Examples:
      | desired_tab |
      |   styles    |
      |   brand     |
      |   layout    |
      |   advanced  |
      |   theme     |

  Scenario: As an administrator, I can add combinatorical permissions
    Given this hasn't been implemented yet

