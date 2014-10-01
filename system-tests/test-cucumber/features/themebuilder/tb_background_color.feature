Feature: Theme Builder background color

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario Outline: As user, I can set the background color of an element
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "<element>"
    When I pick item "<color>" from color picker "style-background-color"
    Then I should see that "<element>" equals "style-background-color" background color
    Examples:
      | element         | color |
      | site name link  | a     |
      | site name link  | c     |
      | site slogan     | b     |
      | site slogan     | d     |

  Scenario Outline: As user, I can set the background color of an element and preview, save and publish theme
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    When I pick item "c" from color picker "style-background-color"
    And I <action> and reload the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    Then I should see that "site name link" equals "style-background-color" background color
    Examples:
      | action  |
      | save    |
      | save_as |
      | publish |

  Scenario Outline: As user, I can set the background image of an element
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "<element>"
    When I set the background image to "herpderphorse.jpg"
    Then I should see that the "herpderphorse.png" background image was uploaded successfully
    And I should see that "<element>" has the correct background image set
    Examples:
      | element         |
      | site name link  |
      | site slogan     |

  Scenario Outline: As user, I can set the background image of an element and save, save as and publish theme
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    When I set the background image to "herpderphorse.jpg"
    And I <action> and reload the current theme
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    Then I should see that "site name link" has the correct background image set
    Examples:
      | action  |
      | save    |
      | save_as |
      | publish |

  Scenario Outline: As user, I can set the background image of an element and set the repetition
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    When I set the background image to "herpderphorse.jpg"
    And I press element "#<repetition>"
    Then I should see that "site name link" has the correct background image set
    Examples:
      | repetition                  |
      | background-repeat-repeat    |
      | background-repeat-repeat-x  |
      | background-repeat-repeat-y  |
      | background-repeat-no-repeat |

  Scenario Outline: As user, I can set the background image of an element and set the attachment
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    When I set the background image to "herpderphorse.jpg"
    And I press element "#<attachment>"
    Then I should see that "site name link" has the correct background image set
    Examples:
      | attachment                    |
      | background-attachment-scroll  |
      | background-attachment-fixed   |

  Scenario: As user, I can remove a set background image
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Background"
    And I select element "site name link"
    And I set the background image to "herpderphorse.jpg"
    When I remove the background image
    Then I should see that "site name link" has the correct background image set
