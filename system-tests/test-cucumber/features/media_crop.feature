Feature: Media crop

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And media crop is enabled

  @utest
  Scenario Outline: As a user, I can add content with media
    When I add a new node
    And I select a media item
    And I "<edit_type>" the image
    And I save the edit
    And I save the node
    Then I can see my media in the node
    Examples:
    | edit_type |
    | crop      |
    | scale     |
    | rotate    |

  @utest
  Scenario Outline: As a user, I can edit content with media
    Given A node with media
    When I edit the node
    And I select media item
    And I "<edit_type>" the image
    And I save the edit
    And I save the node
    Then I can see my media in the node
  Examples:
  | edit_type |
  | crop      |
  | scale     |
  | rotate    |

