Feature: Date module

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And the date module is enabled

  @utest
  Scenario: As a user, I can create a content type containing a date field and widget
    When I select a content type
    And I add a date field
    And I set a widget
    And I set non default date attributes
    And I save my work
    And I create a new piece of content
    Then I can add a date with the expected attributes.

  @utest
  Scenario Outline: As a user, I can change the way the date field behaves
    Given A content type with a date field
    When I edit the date fields settings
    And I chose the "<setting_type>" setting
    And I save my work
    And I edit a piece of content
    Then I can use the new attribute
    Examples:
    |    setting_type    |
    | date entry         |
    | default values     |
    | number of values   |
    | date attributes    |
    | time zone handling |
    | repeating date     |


  @utest
  Scenario Outline: As a user, I can choose how users view the date
    Given A content type with a date field
    When I edit the date fields display settings
    And I select a "<setting_type>"
    And I save my work
    Then I can see the field display changes on the content
    Examples:
    | setting_type |
    | label        |
    | format       |

