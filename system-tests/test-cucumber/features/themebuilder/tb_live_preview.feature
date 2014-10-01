Feature: Theme Builder live preview

  Background:
    Given a fresh gardens installation
    And a new user "previewer" with the password "foo#bar#baz"
    And the role "themer " has the permission "themebuilder preview"
    And I am logged in as user "preview" with the password "foo#bar#baz"

  @smoke, @utest, @wip
  Scenario Outline: As administrator, I can configure the theme preview block to display
    Given the site does not have Themebuilder preview enabled
    And I Enable the Themebuilder preview module
    And I Expose the "Theme preview" block in "<desired_block>"
    And I Allow the role "themer" to preview live themes
    And I am logged in as user "preview" with the password "foo#bar#baz"
    Then I can view the "Theme preview" block in "<desired_block>"
    Examples:
      | desired_block |
      |   Sidebar A   |
      |   Sidebar B   |

  @smoke, @utest, @wip
  Scenario Outline: As user, I can preview any theme that has been saved on the site
    Given the role "themer" has the permission "themebuilder preview"
    And I am on "the homepage"
    Then I should see the site in the preview theme
    And I should be notified that I am in preview mode

  @smoke, @utest, @wip
  Scenario Outline: As user, I can preview any theme that has been saved on the site and revert to the live theme
    Given the role "themer" has the permission "themebuilder preview"
    And I am on "the homepage"
    When I select a theme in the "Theme preview" block
    And I click on "view"
    And I click on "<revert_link>"
    Then I should see the site in the live theme
    Examples: 
      | revert_link    |
      |    in_dsm        |
      | in_preview_block |

  @smoke, @utest, @wip
  Scenario Outline: As other_user, I can not preview any theme
    Given someone is previewing a theme other than the live theme
    And I am on "the homepage"
    Then I cannot see the "Theme preview" block
    And I cannot see any evidence that a theme other than the live theme is active

  @smoke, @utest, @wip
  Scenario Outline: As themer_user I can use ThemeBuilder while user is previewing a saved theme
    Given I am logged in as themer_user
    And I have permissions to theme the site
    And I make changes to the theme and save theme
    Given that someone is logged in as user "previewer" elsewhere
    And they are view the theme I am editing and saving
    Then they can see the theme changes as I save my theme

  @smoke, @utest, @wip
  Scenario Outline: As themer_user I can not use ThemeBuilder while I am previewing a theme
    Given I am logged in as themer_user
    And I have permissions to theme the site
    And I have permissions to "theme preview"
    And I seiect a theme in the "Theme preview" block
    And I open the ThemeBuilder
    Then I should be blocked from opening the ThemeBuilder
    
  @smoke, @utest, @wip
  Scenario Outline: As themer_user I can not preview a theme while I am using the  ThemeBuilder
    Given I am logged in as themer_user
    And I have permissions to theme the site
    And I have permissions to "theme preview"
    And I open the ThemeBuilder
    And I seiect a theme in the "Theme preview" block
    Then I should be blocked from previewing a different theme

  @utest, @wip
  Scenario Outline: As a user with "<site_type>" I "<can_preview>" enable Themebuilder preview
    Given that I have a site of type "<site_type>"
    And I want to enable "Theme preview"
    Then I "<can_preview>"
    Examples:
    | site_type        | can_preview |
    | smb_starter      | can not     |
    | smb_basic        | can         |
    | smb_pro          | can         |
    | smb_premium      | can         |
    | smb_unlimited    | can         |
    | smb_gratis       | can         |
    | enterprise       | can         |