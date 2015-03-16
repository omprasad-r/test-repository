Feature: Theme Builder layouts

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @utest
  Scenario Outline: As user, I can set the "all pages layout" correctly
    Given I am on "the homepage"
    And I open the theme builder
    And I select layout <desired_layout> for all pages
    Then I should see layout <desired_layout> on "the homepage"
    And I should see layout <desired_layout> on "the about page"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

  @utest @selenium
  Scenario Outline: As user, I can set the "this page layout" correctly
    Given I am on "the about page"
    And I open the theme builder
    And I select layout abc for all pages
    And I select layout <desired_layout> for this page
    Then I should see layout <desired_layout> on "the about page"
    And I should see layout abc on "the homepage"
    And I should see layout abc on "the sample blog post"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

      # known issue: DG-2646
  @utest @wip @regression
  Scenario: As user, I can set the "this page layout" for double slashed urls correctly
    Given I am on "//node/2"
    And I open the theme builder
    And I select layout abc for all pages
    And I select layout cab for this page
    Then I should see layout cab on "the about page"
    And I should see layout abc on "the homepage"
    And I should see layout abc on "the sample blog post"

  @utest
  Scenario Outline: As user, I can set, save and reload the "all pages layout" correctly
    Given I am on "the about page"
    And I open the theme builder
    And I select layout <desired_layout> for all pages
    And I save the current theme as "theme_<desired_layout>"
    And I load the theme "theme_<desired_layout>"
    Then I should see layout <desired_layout> on "the homepage"
    And I should see layout <desired_layout> on "the about page"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

      # Test is a bit flaky under webkit
  @utest @selenium
  Scenario Outline: As user, I can set, save and reload the "this page layout" correctly
    Given I am on "the about page"
    And I open the theme builder
    And I select layout abc for all pages
    And I select layout <desired_layout> for this page
    And I save the current theme as "theme_<desired_layout>"
    And I load the theme "theme_<desired_layout>"
    Then I should see layout abc on "the homepage"
    And I should see layout <desired_layout> on "the about page"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

  @utest
  Scenario Outline: As user, I can set, publish and reload the "all pages layout" correctly
    Given I am on "the about page"
    And I open the theme builder
    And I select layout <desired_layout> for all pages
    And I publish the current theme
    Then I should see layout <desired_layout> on "the homepage"
    And I should see layout <desired_layout> on "the about page"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

  @utest @selenium
  Scenario Outline: As user, I can set, publish and reload the "this page layout" correctly
    Given I am on "the about page"
    And I open the theme builder
    And I select layout abc for all pages
    And I select layout <desired_layout> for this page
    And I publish the current theme
    Then I should see layout abc on "the homepage"
    And I should see layout <desired_layout> on "the about page"
    Examples:
      | desired_layout |
      |    abc         |
      |    acb         |
      |    cab         |
      |    ac          |
      |    bc          |
      |    ca          |
      |    cb          |
      |    c           |

  @utest
  Scenario: As a user, I can undo and redo layout selection
    Given this hasn't been automated yet

  @utest
  Scenario: As a user, I can select different layouts with different themes
    Given this hasn't been automated yet

