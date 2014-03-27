Feature: Blocks

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And I am an administrator on the site

  Scenario: As a user, I can create a block
    When I open my blocks management interface
    And I try to add a block
    And I input the required data
    And I save
    Then I have a block

  Scenario: As a user, I can assign a block to a region
    Given I have created a block
    And I assign the block to a region
    Then the block is displayed in the proper region

  Scenario Outline: As a user, I can edit a block
    Given I have created a block assigned to a region
    And I configure it from the <edit mode>
    And I make edits
    Then those changes are reflected when I am done editing
    Examples:
    | edit mode |
    | contextual menu |
    | blocks dialog   |

  Scenario: As a user, I can delete a block
    Given I have created a block assigned to a region
    And I choose to delete the block
    Then my block is gone


  Scenario: As a user, I can assign multiple blocks to a region
    Given I have created a block assigned to a region
    And I create another block and assign it to the same region
    And I create another block and assign it to the same region
    Then I see all my created blocks in the assigned region
    
