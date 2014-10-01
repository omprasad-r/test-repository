Feature: WYSIWYG rich text editor

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a user, I can correctly embed an image in a text
    Given I start creating a page
    And I fill in 'Title' with "Rich text wysiwyg test blah"
    And I enter a text with 200 words and 8 paragraphs into the rich text editor
    When I add the "hurricane-from-space-satellite_w128.jpg" image to the rich text editor
    Then I should see the "hurricane-from-space-satellite_w128.jpg" image embedded in the rich text editor

  Scenario: As a user, I can correctly embed an image in a text and switch between plain and rich text mode
    Given I start creating a page
    And I fill in 'Title' with "Rich text wysiwyg switching test"
    And I enter a text with 200 words and 8 paragraphs into the rich text editor
    When I add the "hurricane-from-space-satellite_w128.jpg" image to the rich text editor
    And I disable the rich-text editor
    And I enable the rich-text editor
    Then I should see the "hurricane-from-space-satellite_w128.jpg" image embedded in the rich text editor

  Scenario: As a user, I can correctly embed an image and save the content
    Given I start creating a page
    And I fill in 'Title' with "Rich text wysiwyg saving test"
    And I enter a text with 200 words and 8 paragraphs into the rich text editor
    When I add the "efd.gif" image to the rich text editor
    And I press "edit-submit"
    Then I should see a status message with the text "Basic page Rich text wysiwyg saving test has been created"
    And I should see the "efd.gif" thumbnail embedded in the page

  Scenario: As a user, I can correctly embed an image and keep when saving
    Given I start creating a page
    And I fill in 'Title' with "Rich text wysiwyg saving and edit test"
    And I enter a text with 200 words and 8 paragraphs into the rich text editor
    When I add the "hurricane-from-space-satellite_w128.jpg" image to the rich text editor
    And I press "edit-submit"
    And I edit the page
    Then I should see the "hurricane-from-space-satellite_w128.jpg" image embedded in the rich text editor

  Scenario: As a user, I can embed and resize an image using handles
    Given this hasn't been implemented yet

  Scenario: As a user, I can embed an image and replace it with another one
    Given this hasn't been implemented yet

  Scenario: As a user, I can embed an image and set the size and type using the options dialog
    Given this hasn't been implemented yet

