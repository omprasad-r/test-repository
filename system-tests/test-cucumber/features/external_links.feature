Feature: External links
  So that users to my site know thye are about to leave when
  They clink on anchors that go off my domian I want to
  Inform them.

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And The external links module is enabled

  @utest
  Scenario Outline: As a user, I can edit the external links configuration
    When I edit the external links configuration
    And I change the '<basic attribute>'
    And a visitor clicks on the external link
    Then They are properly warned about the external link
    Examples:
    | basic attribute |
    | icon on link    |
    | icon on mailto  |
    | new window      |
    | Don't warn      |
    | Warn with page  |
    | Warn with modal |
    | Warn with popup |

  @utest
  Scenario: As a user, I can classify some links as external or internal
    When I edit the external links configuration
    And I change the exclude and include patterns
    And A visitor to my site clicks on a link that matches the pattern
    Then they are properly warned about the external link

  @utest
  Scenario: As a user, I can customize the external link warning message
    When I edit the external links warning message
    And I add custom text
    And I use the available tokens
    And visitor clicks on an external link
    Then they get the custom warning message

  @utest
  Scenario: As a user I can disable external links without breaking my site
    Given I have edited the external link configuration
    And A visitor clicked on an external link
    And gets the proper warning
    When I disable external links
    And A visitor clicks on an external link
    Then they are no longer warned.

