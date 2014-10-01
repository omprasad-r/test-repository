Feature: Theme Builder DOM navigation

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @selenium
  Scenario Outline: As user, I can see the navigation arrows when enabling power theming
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<element>"
    When I enable theme builder power theming
    And I press the theme builder navigation arrow to select the parent element
    Then I should see all theme builder navigation arrows
    Examples:
      | element     |
      | about link  |
      | blog link   |

  @selenium
  Scenario Outline: As user, I can use the theme builder element navigation
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<element>"
    When I enable theme builder power theming
    And I press the theme builder navigation arrow to select the parent element
    And I press the theme builder navigation arrow to select the <selection> element
    Then I should see that the <selection> of "<element>" is selected
    Examples:
      | selection         | element         |
      | parent            | about link      |
      | first child       | site name link  |
      | next sibling      | blog link       |
      | previous sibling  | about link      |

  @selenium
  Scenario Outline: As user, I can see the DOM navigation refiner display
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<element>"
    When I enable theme builder power theming
    Then I should see the DOM navigation refiner display
    Examples:
      | element     |
      | about link  |
      | blog link   |

  @selenium
  Scenario Outline: As user, I can see the correct DOM navigation CSS selector
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "<element>"
    When I enable theme builder power theming
    And I enable theme builder CSS display
    Then I should see that the displayed CSS selector is selected
    Examples:
      | element         |
      | site name link  |
      | about link      |
      | blog link       |

  @selenium
  Scenario: As a user, I can use the DOM navigation to get to the top of the page
    Given I am on the homepage
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "blog link"
    When I enable theme builder power theming
    And I enable theme builder CSS display
    Then I should be able to navigate to the top of the page

  @selenium
  Scenario: As a user, I can use the DOM navigation and change elements
    Given I am on the homepage
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    When I enable theme builder power theming
    And I enable theme builder CSS display
    Then I should be able to navigate to the top of the page and set the following attributes for each element:
      | attribute   | value         |
      | font weight | bold          |
      | font style  | italic        |
      | font family | Bradley Hand  |


  Scenario: As a user, I want don’t want to see the next/previous sibling navigation arrow in the themebuilder if the selected element doesn’t have a next/previous sibling
    Given this hasn't been implemented yet

  Scenario: As a user, I want don’t want to see the child navigation arrow in the themebuilder if the selected element doesn’t have a child sibling
    Given this hasn't been implemented yet

  Scenario: As a user, I can theme elements using a CSS pseudo class
    Given this hasn't been implemented yet

  Scenario: As a user, I can save a themed pseudo class
    Given this hasn't been implemented yet

  Scenario: As a user, I can publish a themed pseudo class
    Given this hasn't been implemented yet

